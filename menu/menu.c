#include "menu.h"

Menu *menu_create(char x, char y, char width, char size){
    char i;
    Menu *menu;
    
    menu = malloc(sizeof(Menu) + sizeof(MenuItem)*size);

    if(!menu)
        return 0x0;

    menu->x=x;
    menu->y=y;
    menu->width=width;
    menu->size=size;
    menu->cursor=0;
    menu->window_start = 0;
    for(i=0;i<size;i++){
        menu->items[i].icon='o';
        menu->items[i].display_text=0x0;
        menu->items[i].func=0x0;
    }
    return menu;
}

void menu_free(Menu *menu, char spare_strings){
    int i;
    if(!menu)
        return;
    if(!spare_strings)
        for(i=0;i<menu->size;i++)
            if(menu->items[i].display_text)
                free(menu->items[i].display_text);
    free(menu);
}

void menu_select(Menu *menu, char pos){
    if(!menu)
        return;
    if(pos < 0 || pos >= menu->size)
        return;
    if(pos < menu->window_start)
        menu->window_start = pos;
    if(pos >= menu->window_start+menu->width)
        menu->window_start = pos-menu->width+1;
    menu->cursor = pos;
}

void menu_activate(Menu *menu){
    if(!menu)
        return;
    if(menu->items[menu->cursor].func)
        menu->items[menu->cursor].func(menu->items[menu->cursor].icon, menu->items[menu->cursor].display_text);
}

void menu_register(Menu *menu, char i, char icon, char *display, void (*func)(char, char *)){
    menu->items[i].icon = icon;
    menu->items[i].display_text = display;
    menu->items[i].func = func;
}

// this must be free'd by caller
char *menu_window(Menu *menu){
    char *out, i, str_size;
    if(!menu)
        return 0x0;
    str_size = menu->width > menu->size ? menu->size : menu->width;
    out = malloc(sizeof(char)*str_size+1);
    for(i=0;i<str_size;i++){
        if(menu->window_start+i==menu->cursor)
            out[i] = 'X';
        else
            out[i] = menu->items[menu->window_start+i].icon;
    }
    out[str_size] = '\0';
    return out;
}

char *menu_message(Menu *menu){
    if(!menu)
        return 0x0;
    return menu->items[menu->cursor].display_text;   
}