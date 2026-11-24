/* SD CEARA MULTIJOGOS V0.6 - USB diagnostics, experimental.
 * USB IRX embedded at build time; PS2 hardware validation required.
 */
#include <tamtypes.h>
#include <kernel.h>
#include <debug.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <libpad.h>
#include <fileXio_rpc.h>
#include <sbv_patches.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

extern unsigned char iomanx_irx[], filexio_irx[], usbd_irx[];
extern unsigned char bdm_irx[], bdmfs_fatfs_irx[], usbmass_bd_irx[];
extern unsigned int size_iomanx_irx, size_filexio_irx, size_usbd_irx;
extern unsigned int size_bdm_irx, size_bdmfs_fatfs_irx, size_usbmass_bd_irx;
static unsigned char padbuf[256] __attribute__((aligned(64)));
typedef struct {const char *name; const char *elf;} Item;
static const Item items[] = {
 {"MEGA DRIVE","mass:/SDCEARA/MULTIJOGOS/EMULADORES/PGEN.ELF"},
 {"SUPER NINTENDO","mass:/SDCEARA/MULTIJOGOS/EMULADORES/SNES.ELF"},
 {"GAME BOY","mass:/SDCEARA/MULTIJOGOS/EMULADORES/GAME_BOY.ELF"},
 {"GAME BOY ADVANCE","mass:/SDCEARA/MULTIJOGOS/EMULADORES/TEMPGBA.ELF"},
 {"ATARI 2600","mass:/SDCEARA/MULTIJOGOS/EMULADORES/ATARI_2600.ELF"},
 {"MASTER SYSTEM","mass:/SDCEARA/MULTIJOGOS/EMULADORES/MASTER_SYSTEM.ELF"},
 {"SNESTICLE (ALT)","mass:/SDCEARA/MULTIJOGOS/EMULADORES/SNESTICLE.ELF"},
 {"SUPER MARIO 64","mass:/SDCEARA/MULTIJOGOS/EMULADORES/SM64.ELF"}
};
#define COUNT ((int)(sizeof(items)/sizeof(items[0])))
static char notice[128]="Iniciando...";
static int usb_status=-999;
static int load_irx(const char *name, void *buf, unsigned int size) {
 int modres=0;
 int ret=SifExecModuleBuffer(buf,size,0,NULL,&modres);
 scr_printf("  %-12s id=%d res=%d\n",name,ret,modres);
 return ret;
}
static void init_usb(void) {
 int r;
 scr_clear(); scr_printf("SD CEARA - INICIALIZACAO USB V0.6\n\n");
 SifInitIopHeap();
 SifLoadFileInit();
 sbv_patch_enable_lmb();
 load_irx("iomanX",iomanx_irx,size_iomanx_irx);
 load_irx("fileXio",filexio_irx,size_filexio_irx);
 r=fileXioInit();
 scr_printf("  fileXioInit=%d\n",r);
 load_irx("usbd",usbd_irx,size_usbd_irx);
 load_irx("bdm",bdm_irx,size_bdm_irx);
 load_irx("bdmfs_fatfs",bdmfs_fatfs_irx,size_bdmfs_fatfs_irx);
 load_irx("usbmass_bd",usbmass_bd_irx,size_usbmass_bd_irx);
 sleep(3);
 usb_status=fileXioOpen("mass:/",O_RDONLY);
 if(usb_status>=0) fileXioClose(usb_status);
 snprintf(notice,sizeof(notice),"USB teste raiz: retorno %d (pode ser diretorio)",usb_status);
 sleep(2);
}
static void draw(int sel) {
 int i; scr_clear();
 scr_printf("\n  SD CEARA GAMES | MULTIJOGOS V0.6\n");
 scr_printf("  Seu Bairro Joga Aqui!\n\n");
 for(i=0;i<COUNT;i++) scr_printf("  %s %s\n",sel==i?">":" ",items[i].name);
 scr_printf("\n  DIRECIONAL: selecionar  X: testar ELF\n");
 scr_printf("  CAMINHO: %s\n",items[sel].elf);
 scr_printf("  %s\n",notice);
}
int main(void) {
 int sel=0,old=0,redraw=1,state;
 SifInitRpc(0); init_scr(); init_usb();
 padInit(0);
 if(!padPortOpen(0,0,padbuf)) {scr_printf("ERRO PAD\n");SleepThread();}
 for(;;) {
   struct padButtonStatus b; int now=0,pressed=0;
   if(redraw){draw(sel);redraw=0;}
   state=padGetState(0,0);
   if((state==PAD_STATE_STABLE||state==PAD_STATE_FINDCTP1)&&padRead(0,0,&b)) {
     now=(~b.btns)&0xffff; pressed=now&~old;old=now;
   } else old=0;
   if(pressed&PAD_DOWN){sel=(sel+1)%COUNT;redraw=1;}
   if(pressed&PAD_UP){sel=(sel+COUNT-1)%COUNT;redraw=1;}
   if(pressed&PAD_CROSS){
     int fd=fileXioOpen(items[sel].elf,O_RDONLY);
     if(fd<0){
       snprintf(notice,sizeof(notice),"USB/ELF: fileXioOpen retornou %d",fd);
     } else {
       fileXioClose(fd);
       snprintf(notice,sizeof(notice),"ELF ENCONTRADO! Tentando executar...");
       draw(sel);sleep(1);
       padPortClose(0,0);padEnd();
       LoadExecPS2(items[sel].elf,0,NULL);
       padInit(0);padPortOpen(0,0,padbuf);
       snprintf(notice,sizeof(notice),"LoadExecPS2 retornou: revisar loader USB");
     }
     redraw=1;
   }
   usleep(90000);
 }
 return 0;
}
