idf_component_register(
    SRCS 
        "main.c" 
        "LCD_Driver/Vernon_ST7789T/Vernon_ST7789T.c" 
        "LCD_Driver/ST7789.c"
        "LVGL_Driver/LVGL_Driver.c"
        "Pomodoro/pomotask.c"
        "ui/ui.c"
        "ui/screens.c"
        "ui/styles.c"
        "vars_ui.c"
        "Task/task_ui.c"
        "ui/ui_font_material_icons.c"
        "ui/ui_font_material_icons_2.c"
        "ui/ui_font_material_icons_1_32.c"

    INCLUDE_DIRS 
        "."
        "./ui"
        "./LCD_Driver/Vernon_ST7789T" 
        "./LCD_Driver" 
        "./LVGL_Driver" 
        "./Pomodoro"
        "./Task"
    REQUIRES 
        Wifi_Connect
        esp_lcd
        Api_Client
        WebSocketClient
        ADC_Lib
)




