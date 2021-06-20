#include "global.h"
#include "battle_pike.h"
#include "battle_pyramid.h"
#include "battle_pyramid_bag.h"
#include "bg.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "event_object_lock.h"
#include "event_scripts.h"
#include "fieldmap.h"
#include "field_effect.h"
#include "field_message_box.h"
#include "field_player_avatar.h"
#include "field_specials.h"
#include "field_special_scene.h"
#include "field_weather.h"
#include "field_screen_effect.h"
#include "frontier_pass.h"
#include "frontier_util.h"
#include "gpu_regs.h"
#include "international_string_util.h"
#include "item_menu.h"
#include "link.h"
#include "load_save.h"
#include "main.h"
#include "menu.h"
#include "new_game.h"
#include "option_menu.h"
#include "overworld.h"
#include "palette.h"
#include "party_menu.h"
#include "pokedex.h"
#include "pokenav.h"
#include "safari_zone.h"
#include "save.h"
#include "scanline_effect.h"
#include "script.h"
#include "sound.h"
#include "sprite.h"
#include "start_menu.h"
#include "strings.h"
#include "string_util.h"
#include "task.h"
#include "text.h"
#include "text_window.h"
#include "trainer_card.h"
#include "window.h"
#include "constants/songs.h"
#include "union_room.h"
#include "constants/rgb.h"
#include "sprite.h"
#include "graphics.h"
#include "malloc.h"
#include "menu_helpers.h"
#include "gpu_regs.h"
#include "decompress.h"
#include "random.h"
#include "window.h"
#include "text_window.h"
#include "io_reg.h"
#include "trainer_pokemon_sprites.h"

enum{
    NORMAN_INIT,
    NORMAN_MESSAGE,
    NORMAN_END
};

//struct encargado en los parametros de la escena de norman, como la funcion que se ejecuta al salir
struct NormanScene{
    MainCallback exitCallback;
    u8 tilemapBuffers[4][BG_SCREEN_SIZE];
    u16 unk;
    u8 animId;
};

//Lo necesitamos para llamar luego al exitCallback del struct NormanScene
static EWRAM_DATA struct NormanScene *sNormanScene = NULL;
EWRAM_DATA static u8 SpriteID = 0;
EWRAM_DATA static u8 SpriteID2 = 0;
/*
Definimos las rutas de los bgs y de los sprite
*/


static const u32 gBg1_NormanFondoTiles[] = INCBIN_U32("graphics/norman_scene/NormanFondoTiles.4bpp.lz"); //Fondo estático
static const u32 gBg1_NormanFondoTilemap[] = INCBIN_U32("graphics/norman_scene/NormanFondoTileMap.bin.lz");
static const u16 gBg1_NormanFondoPalette[] = INCBIN_U16("graphics/norman_scene/NormanFondoPalette.gbapal");

//static const u32 gBg2_CaminoTiles[] = INCBIN_U32("graphics/norman_scene/CaminoTiles.4bpp.lz"); //Camino que se repite constantemente
//static const u32 gBg2_CaminoTilemap[] = INCBIN_U32("graphics/norman_scene/CaminoTilemap.bin.lz");
//static const u16 gBg2_CaminoPalette[] = INCBIN_U16("graphics/norman_scene/CaminoPalette.gbapal"); 

static const u32 gSprite_NormanCarasTiles[] = INCBIN_U32("graphics/norman_scene/NormanCarasTiles.4bpp.lz"); //Mirada hacia Brendan (jugador)
static const u32 gSprite_NormanCarasPalette[] = INCBIN_U32("graphics/norman_scene/NormanFondoPalette.gbapal.lz"); // usados en cara 1 y cara 2

/**fixeo cara*/

static const u32 gSprite_NormanCaras2Tiles[] = INCBIN_U32("graphics/norman_scene/NormanCaras2Tiles.4bpp.lz");

/**
 * Definimos funciones de ejecución
 */
//          DoNormanScene //Fase 0
static void CB2_BeforeInitNormanMenu(void); 
static void CB2_InitNormanMenu(void);
static void CB2_InitNormanMenu2(void);
static void CB2_NormanScene(void); //callback fase 2
static void InitMyTextWindows();
static void InitMyTextsEmerald();
void Task_timerVibracion1_restar(u8 taskId);
void Task_timerVibracion2_restar(u8 taskId);
void vibracionSpritesNorman1 (struct Sprite *sprite);
void vibracionSpritesNorman2 (struct Sprite *sprite);
void Task_FijarTimer1_Callback(u8 taskId);
void Task_RestarTimer1_Callback(u8 taskId);
void Task_FijarTimer2_Callback(u8 taskId);
void Task_RestarTimer2_Callback(u8 taskId);

const u8 gText_Emerald[] = _("EMERALD");
bool8 StartNormanMenu_CB2(void);
enum {TITLE_WINDOW};

/*
Definimos los oam de los sprites, los BgTemplate, los SpriteTemplate, las animaciones de los sprites...
*/
static const struct WindowTemplate gNormanWindowTemplate[] =
{   
   [TITLE_WINDOW] ={
        //Cuando aún no aparece norman
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 15,
        .width = 27,
        .height = 4,
        .paletteNum = 15,
        .baseBlock = 1
    },
    DUMMY_WIN_TEMPLATE
};

static const struct BgTemplate gNormanBackgroundTemplate [] = { 
    {
        .bg = 0,
        .charBaseIndex = 0,//2
        .mapBaseIndex = 2,//31
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,//0
        .baseTile = 0  
    },
    {
        .bg = 1,
        .charBaseIndex = 1,//2
        .mapBaseIndex = 10,//26
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,//0
        .baseTile = 0  
    },
    {
        //fondo 2
        .bg = 2,
        //Dirección de la RAM de video (VRAM) donde se crean/descomprimen los tiles
        //Hay 4 direcciones para almacenar los BG y una más para los sprites, total 5
        .charBaseIndex = 2,
        //Lo que ocupa la información
        .mapBaseIndex = 22,//antes era 8
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 2,
        .baseTile = 0
    },
    {
        .bg = 3,
        .charBaseIndex = 3,//2
        .mapBaseIndex = 30,//26
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 3,//0
        .baseTile = 0  
    },
};

static const struct CompressedSpriteSheet sSpriteSheet_Mon[] =
{
    /* Esta forma es otra de ponerlo
    .data = gTest_Mon,
    .size = 0x4096,
    .tag = 777
    */
    {gSprite_NormanCarasTiles, 0x3000, 777},
    {NULL},
};

static const struct CompressedSpriteSheet sSpriteSheet_Mon2[] =
{
    /* Esta forma es otra de ponerlo
    .data = gTest_Mon,
    .size = 0x4096,
    .tag = 777
    */
    {gSprite_NormanCaras2Tiles, 0x1000, 778},
    {NULL},
};

static const struct CompressedSpritePalette sSpritePal_Mon[] =
{
    
    {gSprite_NormanCarasPalette, 777},
    {NULL},
    
};

static const struct CompressedSpritePalette sSpritePal_Mon2[] =
{
    
    {gSprite_NormanCarasPalette, 778},
    {NULL},
    
};

static const struct OamData sMonOamData =
{
    .y = 0,
    .affineMode = 0,
    .objMode = 0,
    .mosaic = 0,
    .bpp = 0,//ST_OAM_8BPP para .8bpp
    .shape = SPRITE_SHAPE(64x64),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(64x64),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0,
};

static const struct OamData sMonOamData2 =
{
    .y = 0,
    .affineMode = 0,
    .objMode = 0,
    .mosaic = 0,
    .bpp = 0,//ST_OAM_8BPP para .8bpp
    .shape = SPRITE_SHAPE(64x64),
    .x = 0,
    .matrixNum = 0,
    .size = SPRITE_SIZE(64x64),
    .tileNum = 0,
    .priority = 0,
    .paletteNum = 0,
    .affineParam = 0,
};
static const union AnimCmd smon_AnimNulo[] = //animacion mirando hacia ti
{
    ANIMCMD_FRAME(0, 15),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd smon_AnimNulo2[] = //animacion mirando hacia el paisaje
{
    ANIMCMD_FRAME(192, 15),
    ANIMCMD_JUMP(192),
};

static const union AnimCmd smon_Anim0[] =
{
    ANIMCMD_FRAME(0, 15),
    ANIMCMD_FRAME(64, 15),
    ANIMCMD_FRAME(128, 15),
    ANIMCMD_FRAME(0, 15),
    ANIMCMD_FRAME(64, 15),
    ANIMCMD_FRAME(128, 15),
    ANIMCMD_FRAME(0, 15),
    ANIMCMD_FRAME(64, 15),
    ANIMCMD_FRAME(128, 15),
    ANIMCMD_FRAME(0, 15),
    ANIMCMD_END,
};
// Esto anima el sprite cogiendo los frames, en este caso 15 es la velocidad
static const union AnimCmd smon_Anim1[] =
{
    //primer párametro posición
    //segundo parámetro velocidad
    ANIMCMD_FRAME(0, 15),
    ANIMCMD_FRAME(64, 15),
    ANIMCMD_FRAME(128, 15),
    ANIMCMD_FRAME(192, 15),
    ANIMCMD_FRAME(256, 15),
    ANIMCMD_FRAME(320, 15),
    ANIMCMD_JUMP(0),
};

static const union AnimCmd smon_Anim3[] = //3 parte antes del dialogo
{
    ANIMCMD_FRAME(0, 10),
    ANIMCMD_FRAME(0, 10),
    ANIMCMD_FRAME(0, 10),
    ANIMCMD_FRAME(192, 10),
    ANIMCMD_FRAME(256, 10),
    ANIMCMD_END,
};

static const union AnimCmd smon_Anim4[] = //4 parte durante el dialogo
{
    ANIMCMD_FRAME(256, 15),
    ANIMCMD_FRAME(320, 15),
    ANIMCMD_FRAME(256, 15),
    ANIMCMD_FRAME(320, 15),
    ANIMCMD_FRAME(256, 15),
    ANIMCMD_FRAME(320, 15),
    ANIMCMD_FRAME(256, 15),
    ANIMCMD_FRAME(320, 15),
    ANIMCMD_FRAME(256, 15),
    ANIMCMD_END,
};

static const union AnimCmd smon_Anim5[] = //4 parte durante el dialogo
{
    ANIMCMD_FRAME(128, 10),
   ANIMCMD_FRAME(128, 10),
   ANIMCMD_FRAME(128, 10),
    ANIMCMD_FRAME(64, 10),
    ANIMCMD_FRAME(0, 10),
    ANIMCMD_END,
};

static const union AnimCmd smon_Anim6[] = //mira desde el coche hacia nosotros
{
    ANIMCMD_FRAME(192, 15),
   ANIMCMD_FRAME(0, 15),
   ANIMCMD_FRAME(64, 15),
    ANIMCMD_FRAME(128, 15),
    ANIMCMD_FRAME(64, 15),
    ANIMCMD_FRAME(128, 15),
    ANIMCMD_FRAME(64, 15),
    ANIMCMD_FRAME(128, 15),
    ANIMCMD_FRAME(64, 15),
    ANIMCMD_FRAME(128, 15),
    ANIMCMD_FRAME(64, 15),
    ANIMCMD_FRAME(128, 15),
    ANIMCMD_FRAME(0, 15),
    ANIMCMD_END,
};

static const union AnimCmd smon_Anim6B[] = //mira desde el coche hacia nosotros SPRITE 2
{
   ANIMCMD_FRAME(64, 15),
    ANIMCMD_FRAME(128, 15),
   ANIMCMD_FRAME(128, 15),
    ANIMCMD_FRAME(128, 15),
    ANIMCMD_FRAME(128, 15),
    ANIMCMD_FRAME(128, 15),
    ANIMCMD_FRAME(128, 15),
    ANIMCMD_FRAME(128, 15),
    ANIMCMD_FRAME(128, 15),
    ANIMCMD_FRAME(128, 15),
    ANIMCMD_FRAME(128, 15),
    ANIMCMD_FRAME(128, 15),
    ANIMCMD_FRAME(128, 15),
    ANIMCMD_END,
};

enum {CERO, CERO2, PRIMERO, SEGUNDO, TERCERO, CUARTO, QUINTO, SEXTO,SEXTO2};

static const union AnimCmd *const smon_AnimTable[] =
{
    /*
        IMPORTANTE
    
        - Con StartSpriteAnim puedes hacer las dos cosas:

            StartSpriteAnim(&gSprites[SpriteID], 0); posicion 0
            StartSpriteAnim(&gSprites[SpriteID], PRIMERO); ya que es la posición 0
    */

    [CERO] = smon_AnimNulo,

    [CERO2] = smon_AnimNulo2,

    [PRIMERO] = smon_Anim0,
    //smon_Anim0,
    [SEGUNDO] = smon_Anim1,
    //smon_Anim1,
    [TERCERO] = smon_Anim3,

    [CUARTO] = smon_Anim4,

    [QUINTO] = smon_Anim5,

    [SEXTO] = smon_Anim6,

    [SEXTO2] = smon_Anim6B,
};

static const struct SpriteTemplate sMonSpriteTemplate =
{
    .tileTag = 777,
    .paletteTag = 777,
    .oam = &sMonOamData,
    //.anims = gDummySpriteAnimTable,
    .anims = smon_AnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const struct SpriteTemplate sMonSpriteTemplate2 =
{
    .tileTag = 778,
    .paletteTag = 778,
    .oam = &sMonOamData2,
    //.anims = gDummySpriteAnimTable,
    .anims = smon_AnimTable,
    .images = NULL,
    .affineAnims = gDummySpriteAffineAnimTable,
    .callback = SpriteCallbackDummy,
};

static const u8 sFontColourTable[] = { TEXT_COLOR_TRANSPARENT, TEXT_DYNAMIC_COLOR_1, TEXT_DYNAMIC_COLOR_4};
static void AddText(const u8* text)
{
    StringExpandPlaceholders(gStringVar4, text); //Coloca el texto en el buffer de strings
}

static void CreateTextBox(u8 windowId)
{
    //FillWindowPixelBuffer(windowId, 0);    //Llena todos los pixeles del Buffer con el color indicado (0-> transparencia)
    //PutWindowTilemap(windowId);    //Coloca el tilemap de los gráficos
    //sFontColourTable en el quinto parámetro
    //AddTextPrinterParameterized3(windowId, 1, 0, 0, sFontColourTable, -1, gStringVar4); //Asigna un textPrinter
    //CopyWindowToVram(windowId, 3);    //Copia la ventana del buffer a la VRam
}

static void InitMyTextWindows()
{
    InitWindows(gNormanWindowTemplate);    //Inicializa los window
    DeactivateAllTextPrinters();     //Desactiva cualquier posible Text printer que haya quedado abierto
    LoadPalette(GetOverworldTextboxPalettePtr(), 0xf0, 0x20);    //Carga la paleta del texto por defecto del juego en el slot 15 de las paletas para backgrounds
    //LoadUserWindowBorderGfx(YES_NO_WINDOW, 211, 0xe0); //Opcional, necesario si se van a usar ventanas Yes/no
}

static void InitMyTextsEmerald()
{
    
    //CreateTextBox(TITLE_WINDOW);
    ShowFieldMessage(gText_Norman1);  
}

/**funcion llamada por script a través de un special que ejecuta la escena de norman
 * void (*exitCallback)(void) -> Función que se ejecuta tras terminar la escena de norman para volver al juego
*/
void DoNormanScene(u8 animId, void (*exitCallback)(void))
{
    sNormanScene = AllocZeroed(sizeof(*sNormanScene));//para calcular los datos del struct sNormanScene
    sNormanScene->animId = animId;
    sNormanScene->exitCallback = exitCallback;
    StartNormanMenu_CB2();
}

static void LoadNormanBgs ()
{
    ResetAllBgsCoordinates();
    //ResetBgsAndClearDma3BusyFlags(0); esto arregla el problema del bg1 con el tile 0,0 del .bin
    ResetBgsAndClearDma3BusyFlags(0);
    
    InitBgsFromTemplates(0, gNormanBackgroundTemplate, ARRAY_COUNT(gNormanBackgroundTemplate));



    //0x4000 * charBaseIndex
    LZ77UnCompVram(gBg1_NormanFondoTiles, (void*) VRAM + 0x4000 * 2);
    //BG_SCREEN_ADDR(26) por el mapBaseIndex que es 26
    LZ77UnCompVram(gBg1_NormanFondoTilemap, (u16*) BG_SCREEN_ADDR(22));
    //Cargar paletas del Bg
    LoadPalette(gBg1_NormanFondoPalette, 0x0, 0x20);

   LoadCompressedSpriteSheet(&sSpriteSheet_Mon[0]);
   LoadCompressedSpritePalette(sSpritePal_Mon);
  SpriteID = CreateSprite(&sMonSpriteTemplate, 198, 32, 0);

  LoadCompressedSpriteSheet(&sSpriteSheet_Mon2[0]);
   LoadCompressedSpritePalette(sSpritePal_Mon2);
  //SpriteID2 = CreateSprite(&sMonSpriteTemplate2, 208,32,0);  

    ShowBg(2);
    ShowBg(0);
    //ShowBg(1);
    //ShowBg(3);

    // AddText(gText_MenuTitle);
    // CreateTextBox(TITLE_WINDOW);
    // InitMyTextsEmerald();
}

//Del tutorial
static void VBlank_CB_Norman()
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

//Del tutorial
static void CB2_NormanScene()
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

#define tBG2HOFS data[4]
#define tBG2VOFS data[0]
#define tBG3VOFS data[3]
#define tBG3HOFS data[1]

void Task_OpenPruebaFadeOut(u8 taskId){
    if (!gPaletteFade.active)
    {
        DestroyTask(taskId);
        FreeAllWindowBuffers();
        SetMainCallback2(CB2_BeforeInitNormanMenu);
    }
}

static void CB2_BeforeInitNormanMenu ()
{
   PlaySE(SE_TRUCK_MOVE);
   
   SetMainCallback2(CB2_InitNormanMenu);
}

bool8 StartNormanMenu_CB2()
{
    if(!gPaletteFade.active)
    {
        
        gMain.state = 0;
        //RemoveExtraStartMenuWindows();
        CleanupOverworldWindowsAndTilemaps();
        //del BeginNormalPaletteFade:
        /* primer argumento: paleta. segundo argumento: delay(transición).tercero:target "Y"
        cuarto, quinto: color de transición */
      // BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 0x10, RGB_BLACK);
        BlendPalettesGradually(0xFFFF0000, 1, 16, 0, RGB_BLACK,      0, 1);
        BeginNormalPaletteFade(0xFFFFFFFF, 10, 0, 0x10, RGB_BLACK);
        CreateTask(Task_OpenPruebaFadeOut,0);
        
        
        return TRUE;
    }

    return FALSE;
}

static void Task_NormanFadeOut(u8 taskId){
    if(!gPaletteFade.active){
        DestroyTask(taskId);
        FreeAllWindowBuffers();
        SetMainCallback2(CB2_ReturnToField);
    }
}
static void Task_HandleKeyPressed(u8 taskId){
    if (JOY_NEW(B_BUTTON)){
        PlaySE(SE_TRUCK_STOP);
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 0x10, RGB_BLACK);
        gTasks[taskId].func = Task_NormanFadeOut;
    }
}
#define tTimer data[7]

void Task_FijarTimer1_Callback(u8 taskId){
    gTasks[taskId].tTimer = 0x10;
    gTasks[taskId].func = Task_RestarTimer1_Callback;
}

void Task_RestarTimer1_Callback(u8 taskId){
    if (gTasks[taskId].tTimer){
        gTasks[taskId].tTimer--;
    }else{
        gSprites[SpriteID].callback = vibracionSpritesNorman2;
        gTasks[taskId].func = Task_FijarTimer2_Callback;
    }    
}

void Task_FijarTimer2_Callback(u8 taskId){
    gTasks[taskId].tTimer = 0x10;
    gTasks[taskId].func = Task_RestarTimer2_Callback;
}

void Task_RestarTimer2_Callback(u8 taskId){
    if (gTasks[taskId].tTimer){
        gTasks[taskId].tTimer--;
    }else{
        gSprites[SpriteID].callback = vibracionSpritesNorman1;
        gTasks[taskId].func = Task_FijarTimer1_Callback;
    }    
}

void vibracionSpritesNorman1 (struct Sprite *sprite){
    
    gSprites[SpriteID].pos1.y += 1;
    sprite->pos1.y -= 1; // es lo mismo que: sprite->pos1.y--;
    sprite->pos1.y = 207;
    gSprites[SpriteID].callback = vibracionSpritesNorman2;
    
}
void vibracionSpritesNorman2 (struct Sprite *sprite){
    
    gSprites[SpriteID].pos1.y -= 1;
    sprite->pos1.y += 1;
    sprite->pos1.y = 208;
    gSprites[SpriteID].callback = vibracionSpritesNorman1;
    
}

static void TaskFaseFinal(u8 taskId)
{
    if (gTasks[taskId].tTimer){
        gTasks[taskId].tTimer--;
    }else{
        DestroyTask(taskId);
        FreeAllWindowBuffers();
        SetMainCallback2(CB2_InitNormanMenu2);
    }   
}
static void TaskFaseSiguiente3texto(u8 taskId){
    if(!RunTextPrintersAndIsPrinter0Active()){
        ClearDialogWindowAndFrame(0, 1);
        gTasks[taskId].tTimer= 0xA0;
        gTasks[taskId].func = TaskFaseFinal;
    }
}
static void TaskFaseSiguiente2(u8 taskId){
       gTasks[taskId].func = TaskFaseSiguiente3texto;   
}
static void TaskFaseSiguiente2Texto(u8 taskId){
    gTasks[taskId].func = TaskFaseSiguiente2;       
}
static void TaskFaseSiguiente(u8 taskId)
{
        gTasks[taskId].func = TaskFaseSiguiente2Texto;       
}


static void Task_Texto_Fase2(u8 taskId){ // hijo estas dormido
    
    InitMyTextWindows();
    InitMyTextsEmerald();  
    gTasks[taskId].func = TaskFaseSiguiente;
}

static void TaskFaseDesvanecer2(u8 taskId){
      if (gTasks[taskId].tTimer){
        gTasks[taskId].tTimer--;
    }else{
         BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 0x10, RGB_BLACK); 
         gTasks[taskId].func = TaskFaseSiguiente3texto;
    }    
}
static void TaskFaseDesvanecer(u8 taskId){
    if(!RunTextPrintersAndIsPrinter0Active()){
        ClearDialogWindowAndFrame(0, 1);
        gTasks[taskId].tTimer= 0x60;
        gTasks[taskId].func = TaskFaseDesvanecer2;
    }
    
}

static void TaskTextoNormanTerceraFasee(u8 taskId){
    StartSpriteAnim(&gSprites[SpriteID], CUARTO); 
     InitMyTextWindows();
     ShowFieldMessage(gText_Norman3d);
    gTasks[taskId].func = TaskFaseDesvanecer;
}

static void TaskTextoNormanTerceraFasee1(u8 taskId){
    if (gTasks[taskId].tTimer){
        gTasks[taskId].tTimer--;
    }else{
          
         gTasks[taskId].func = TaskTextoNormanTerceraFasee;
    } 
     
}

static void TaskTextoNormanTerceraFased(u8 taskId){
    if(!RunTextPrintersAndIsPrinter0Active()){
        ClearDialogWindowAndFrame(0, 1);
        gTasks[taskId].tTimer= 0x60;
        gTasks[taskId].func = TaskTextoNormanTerceraFasee1;
    }
}

static void TaskTextoNormanTerceraFasec(u8 taskId){
    StartSpriteAnim(&gSprites[SpriteID], CUARTO); 
     InitMyTextWindows();
     ShowFieldMessage(gText_Norman3c);
    gTasks[taskId].func = TaskTextoNormanTerceraFased;
}
static void TaskTextoNormanTerceraFaseb(u8 taskId){
    if (gTasks[taskId].tTimer){
        gTasks[taskId].tTimer--;
    }else{
          
         gTasks[taskId].func = TaskTextoNormanTerceraFasec;
    } 
     
}

static void TaskTextoNormanTerceraFase(u8 taskId){
    if(!RunTextPrintersAndIsPrinter0Active()){
        ClearDialogWindowAndFrame(0, 1);
         StartSpriteAnim(&gSprites[SpriteID], TERCERO);    
    StartSpriteAnim(&gSprites[SpriteID2], QUINTO);
        gTasks[taskId].tTimer= 0xA0;
        gTasks[taskId].func = TaskTextoNormanTerceraFaseb;
    }
}

static void TaskTextoNormanSegundaFaseb(u8 taskId){
     StartSpriteAnim(&gSprites[SpriteID],SEXTO);
     StartSpriteAnim(&gSprites[SpriteID2],SEXTO2);
     InitMyTextWindows();
     ShowFieldMessage(gText_Norman3b);
    gTasks[taskId].func = TaskTextoNormanTerceraFase;
}
static void TaskTextoNormanSegundaFase(u8 taskId){
    // StartSpriteAnim(&gSprites[SpriteID],CUARTO);
    // ShowFieldMessage(gText_Norman3b);
    // gTasks[taskId].func = TaskFaseSiguiente3texto;
    if(!RunTextPrintersAndIsPrinter0Active()){
        ClearDialogWindowAndFrame(0, 1);
        gTasks[taskId].tTimer= 0xA0;
        gTasks[taskId].func = TaskTextoNormanSegundaFaseb;
    }
}
static void TaskFaseNormanImaginandoTexto(u8 taskId){
    StartSpriteAnim(&gSprites[SpriteID], CUARTO); 
    InitMyTextWindows();
    ShowFieldMessage(gText_Norman3);   
    gTasks[taskId].func = TaskTextoNormanSegundaFase;
}

static void TaskFaseNormanImaginando(u8 taskId){
    if (gTasks[taskId].tTimer){
        gTasks[taskId].tTimer--;
    }else{
           
         gTasks[taskId].func = TaskFaseNormanImaginandoTexto;
    } 
}

static void TaskFaseSiguienteCara2Texto(u8 taskId){
    SpriteID2 = CreateSprite(&sMonSpriteTemplate2, 208,32,0);
    StartSpriteAnim(&gSprites[SpriteID], TERCERO);    
    StartSpriteAnim(&gSprites[SpriteID2], QUINTO);
    gTasks[taskId].tTimer= 0xA0;
    gTasks[taskId].func = TaskFaseNormanImaginando;
}

static void TaskFaseSiguienteCara2(u8 taskId){
     if(!RunTextPrintersAndIsPrinter0Active()){
       ClearDialogWindowAndFrame(0, 1);
        gTasks[taskId].tTimer= 0xA0;
        gTasks[taskId].func = TaskFaseSiguienteCara2Texto;
    }
}

static void TaskFaseSiguienteCara2previo(u8 taskId){
    gTasks[taskId].func = TaskFaseSiguienteCara2;
}    

static void TaskFaseSiguienteCara(u8 taskId){   
    StartSpriteAnim(&gSprites[SpriteID], PRIMERO);
    gTasks[taskId].func = TaskFaseSiguienteCara2previo;
}

static void Task_Texto_Fase4(u8 taskId){ // ha sido un duro dia de mudanzas
    
    InitMyTextWindows();
    ShowFieldMessage(gText_Norman2);   
    gTasks[taskId].func = TaskFaseSiguienteCara;
}

static void Task_Texto_Fase3(u8 taskId){ //texto despues de hijo estas dormido
    if(gTasks[taskId].tTimer){
        gTasks[taskId].tTimer--;
    }else{
        gTasks[taskId].func = Task_Texto_Fase4;
    }
}

static void Task_Texto_Fase(u8 taskId){
    if(gTasks[taskId].tTimer){
        gTasks[taskId].tTimer--;
    }else{
        gTasks[taskId].func = Task_Texto_Fase2;
    }
}
static void Task_TextoLento(u8 taskId){
    gTasks[taskId].tTimer= 0xA0;
    gTasks[taskId].func = Task_Texto_Fase;
}

static void Task_TextoLento2(u8 taskId){ //timer despues de hijo estas dormido
    gTasks[taskId].tTimer= 0x70;
    gTasks[taskId].func = Task_Texto_Fase3;
}

void Task_timerVibracion1_restar(u8 taskId){
    if(gTasks[taskId].tTimer % 1 + Random() % 0xA == 0){
        SetGpuReg(REG_OFFSET_BG2VOFS, gTasks[taskId].tBG2VOFS -= 1);
        gSprites[SpriteID].pos1.y += 1;
        gSprites[SpriteID2].pos1.y += 1;
        gTasks[taskId].func = Task_timerVibracion2_restar;
    }
    gTasks[taskId].tTimer ++;    
}

void Task_timerVibracion2_restar(u8 taskId){
    if(gTasks[taskId].tTimer % 1 + Random() % 0xA == 0){
        SetGpuReg(REG_OFFSET_BG2VOFS, gTasks[taskId].tBG2VOFS += 1);
        gSprites[SpriteID].pos1.y -= 1;
        gSprites[SpriteID2].pos1.y -= 1;
        gTasks[taskId].func = Task_timerVibracion1_restar;
    }
    gTasks[taskId].tTimer ++;  
} 

static void CB2_InitNormanMenu ()  
{
    switch(gMain.state)
    {
        case 0:
            SetVBlankHBlankCallbacksToNull();
            ScanlineEffect_Stop();
            ResetTasks();
            ResetSpriteData();
            ResetPaletteFade();
            FreeAllSpritePalettes();
            DmaClearLarge16(3, (void*)VRAM, VRAM_SIZE, 0x1000);
            gMain.state++;
            break;

        case 1:
           // LoadNormanBgs();
            gMain.state++;
            break;

        case 2:
            SetVBlankCallback(VBlank_CB_Norman);
            SetMainCallback2(CB2_NormanScene);
            CreateTask(Task_TextoLento,0);
            break;
    }
}

static void CB2_InitNormanMenu2 ()  
{
    switch(gMain.state)
    {
        case 0:
            SetVBlankHBlankCallbacksToNull();
            ScanlineEffect_Stop();
            ResetTasks();
            ResetSpriteData();
            ResetPaletteFade();
            FreeAllSpritePalettes();
            DmaClearLarge16(3, (void*)VRAM, VRAM_SIZE, 0x1000);
            gMain.state++;
            break;

        case 1:
            
            LoadNormanBgs();
            CreateTask(Task_timerVibracion1_restar, 0);
            gMain.state++;
            break;

        case 2:
            SetVBlankCallback(VBlank_CB_Norman);
            FadeInFromBlack();

            //StartSpriteAnim(&gSprites[SpriteID], SEGUNDO);
            //StartSpriteAnim(&gSprites[SpriteID2], TERCERO);
            SetMainCallback2(CB2_NormanScene);
            CreateTask(Task_TextoLento2,0);
            CreateTask(Task_HandleKeyPressed,0);
            break;
    }
}

