/* SD CEARA MULTIJOGOS - prototype for PS2SDK.
 * NOT YET COMPILED OR VALIDATED ON REAL HARDWARE.
 * Requires an operational USB mass driver before LoadExecPS2(mass:/...).
 */
#include <tamtypes.h>
#include <kernel.h>
#include <debug.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <libpad.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static unsigned char padbuf[256] __attribute__((aligned(64)));
typedef struct { const char *name; const char *elf; } Item;
static const Item items[] = {
 {"MEGA DRIVE", "mass:/SDCEARA/MULTIJOGOS/EMULADORES/PGEN.ELF"},
 {"SUPER NINTENDO", "mass:/SDCEARA/MULTIJOGOS/EMULADORES/SNES.ELF"},
 {"GAME BOY", "mass:/SDCEARA/MULTIJOGOS/EMULADORES/GAME_BOY.ELF"},
 {"GAME BOY ADVANCE", "mass:/SDCEARA/MULTIJOGOS/EMULADORES/TEMPGBA.ELF"},
 {"ATARI 2600", "mass:/SDCEARA/MULTIJOGOS/EMULADORES/ATARI_2600.ELF"},
 {"MASTER SYSTEM", "mass:/SDCEARA/MULTIJOGOS/EMULADORES/MASTER_SYSTEM.ELF"},
 {"SNESTICLE (ALT)", "mass:/SDCEARA/MULTIJOGOS/EMULADORES/SNESTICLE.ELF"},
 {"SUPER MARIO 64", "mass:/SDCEARA/MULTIJOGOS/EMULADORES/SM64.ELF"}
};
#define NITEMS (sizeof(items)/sizeof(items[0]))
static void draw(int selected, const char *notice) {
 int i;
 scr_clear();
 scr_printf("\n     SD CEARA GAMES  |  MULTIJOGOS\n");
 scr_printf("     Seu Bairro Joga Aqui!\n");
 scr_printf("     -------------------------------\n\n");
 for (i=0;i<(int)NITEMS;i++)
   scr_printf("     %s %s\n", selected==i?">":" ", items[i].name);
 scr_printf("\n     DIRECIONAL: SELECIONAR  |  X: ABRIR\n");
 scr_printf("\n     %s\n", notice ? notice : "PROTOTIPO DE NAVEGACAO");
}
int main(int argc, char **argv) {
 int selected=0, old=0, state, connected=0, redraw=1;
 const char *notice="USB deve estar inicializado antes de abrir um ELF";
 (void)argc; (void)argv;
 SifInitRpc(0);
 init_scr();
 padInit(0);
 if (padPortOpen(0,0,padbuf)==0) {
   scr_printf("Falha ao iniciar controle; verifique modulos PAD.\n");
   SleepThread();
 }
 for (;;) {
   struct padButtonStatus buttons;
   int pressed=0, now;
   if (redraw) { draw(selected,notice); redraw=0; }
   state=padGetState(0,0);
   connected=(state==PAD_STATE_STABLE || state==PAD_STATE_FINDCTP1);
   if (connected && padRead(0,0,&buttons)!=0) {
     now=(~buttons.btns)&0xffff;
     pressed=now & ~old;
     old=now;
   } else old=0;
   if (pressed & PAD_DOWN) { selected=(selected+1)%NITEMS; redraw=1; }
   if (pressed & PAD_UP) { selected=(selected+NITEMS-1)%NITEMS; redraw=1; }
   if (pressed & PAD_CROSS) {
     /* The USB mass device must be available to the EE loader.
        A separate USB/IOP initialization stage is still required. */
     FILE *fp=fopen(items[selected].elf,"rb");
     if (!fp) { notice="ELF inacessivel: verifique USB/driver/caminho"; redraw=1; }
     else {
       fclose(fp);
       scr_clear();
       scr_printf("Abrindo %s...\n",items[selected].name);
       padPortClose(0,0);
       padEnd();
       LoadExecPS2(items[selected].elf,0,NULL);
       notice="Falha ao iniciar ELF: revisar metodo de carregamento"; redraw=1;
       padInit(0);
       padPortOpen(0,0,padbuf);
     }
   }
   usleep(90000);
 }
 return 0;
}
