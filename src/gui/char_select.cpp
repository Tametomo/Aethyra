/**

        The Mana World
        Copyright 2004 The Mana World Development Team

    This file is part of The Mana World.

    The Mana World is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    The Mana World is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with The Mana World; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

		By ElvenProgrammer aka Eugenio Favalli (umperio@users.sourceforge.net)

*/

#include "char_select.h"
#include "../graphic/graphic.h"

char button_state[3];
char address[41];
char name[25];

DIALOG char_select_dialog[] = {
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)                    (d2)  (dp)                          (dp2) (dp3) */
   { tmw_dialog_proc,   300,  240,  200,  208,    0,  -1,    0,    0,          0,                 0,   (char*)"Char select",                      NULL, NULL  },
   { tmw_text_proc,     308,  268,  192,   10,    0,   0,    0,    0,          0,                 0,                   NULL,                      NULL, NULL  },
   { tmw_bitmap_proc,   304,  282,  192,   70,    0,   0,    0,    0,          80,                60,             playerset,                      NULL, NULL  },
   { tmw_button_proc,   398,  426,   44,   18,    0,  -1,    'o',  D_EXIT,    -1,                 0,           (char*)"&Ok",                      NULL, NULL  },
   { tmw_button_proc,   446,  426,   44,   18,    0,  -1,    'c',  D_EXIT,    -1,                 0,       (char*)"&Cancel",                      NULL, NULL  },
   { tmw_button_proc,   304,  426,   44,   18,    0,   0,    0,    D_EXIT,     0,                 0,           button_state,                      NULL, NULL  },
/* { gui_button_proc,   304,  356,   20,   20,    0,   0,    0,    0,          0,                 0,                    "<",                      NULL, NULL  },
   { gui_button_proc,   328,  356,   20,   20,    0,   0,    0,    0,          0,                 0,                    ">",                      NULL, NULL  },  */
   { NULL,                0,    0,   0,     0,    0,   0,    0,    0,          0,                 0,                   NULL,                      NULL, NULL  },
};

DIALOG char_create_dialog[] = {
   /* (dialog proc)     (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)     (d1)                    (d2)  (dp)                          (dp2) (dp3) */
   { tmw_dialog_proc,   276,  240,  248,  116,    0,  -1,    0,    0,          0,                 0,   (char*)"Char create",                      NULL, NULL  },
   { tmw_text_proc,     280,  268,  192,   20,    0,   0,    0,    0,          0,                 0,       (char *)"Name: ",                      NULL, NULL  },
   { tmw_edit_proc,     336,  268,  162,   20,    0,   0,    0,    0,         24,                 0,                   name,                      NULL, NULL  },
   { tmw_button_proc,   398,  334,   44,   18,    0,  -1,    'o',  D_EXIT,    -1,                 0,           (char*)"&Ok",                      NULL, NULL  },
   { tmw_button_proc,   446,  334,   44,   18,    0,  -1,    'c',  D_EXIT,    -1,                 0,       (char*)"&Cancel",                      NULL, NULL  },
   { NULL,                0,    0,   0,     0,    0,   0,    0,    0,          0,                 0,                   NULL,                      NULL, NULL  },
};

void char_select() {
  state = LOGIN;
  if(n_character>0) {
    char_select_dialog[1].dp = char_info->name;
		char_select_dialog[1].x = 400-alfont_text_length(gui_font, char_info->name)/2;
		char_select_dialog[2].dp = playerset;
		if(playerset==NULL)ok("Error", "Playerset not loaded");
		strcpy(button_state, "Del");
		char_select_dialog[3].flags &= D_CLOSE;
	} else {
		char *temp = (char *)malloc(3);
		strcpy(temp, "");
		char_select_dialog[1].dp = temp;
		char_select_dialog[3].flags |= D_DISABLED;
		char_select_dialog[2].dp = NULL;
		strcpy(button_state, "New");
	}
	//centre_dialog(char_select_dialog);
	DIALOG_PLAYER *player = init_dialog(char_select_dialog, -1);
	int gui_exit = 1;
	while ((!key[KEY_ESC])&&(gui_exit)&&(!key[KEY_ENTER])) {
		clear_bitmap(buffer);
		blit((BITMAP *)graphic[LOGIN_BMP].dat, buffer, 0, 0, 0, 0, 800, 600);
		gui_exit = gui_update(player);
		blit(buffer, screen, 0, 0, 0, 0, 800, 600);
	}
	gui_exit = shutdown_dialog(player);
	if((gui_exit==3)||(key[KEY_ENTER]))server_char_select();
	else if(gui_exit==4)close_session();
	else if(gui_exit==5)server_char_delete();
	if(state==LOGIN)close_session();
}

void server_char_select() {
  // Request character selection
  WFIFOW(0) = net_w_value(0x0066);
  WFIFOB(2) = net_b_value(0);
  WFIFOSET(3);

  while((in_size<3)||(out_size>0))flush();
  log_hex("Char_Select_Packet", "Packet_ID", RFIFOW(0));
  log_int("Char_Select_Packet", "Packet_length", get_length(RFIFOW(0)));
  log_int("Char_Select_Packet", "Packet_in_size", RFIFOW(2));
  log_int("Char_Select_Packet", "In_size", in_size);

  if(RFIFOW(0)==0x0071) {
    while(in_size<28)flush();
    char_ID = RFIFOL(2);
    memset(map_path, '\0', 480);
    append_filename(map_path, "./data/map/", RFIFOP(6), 480);
    map_address = RFIFOL(22);
    map_port = RFIFOW(26);
    state = GAME;

    log("Player", "map", map_name);
    log("Char_Select_packet", "server_address", iptostring(map_address));
    log_int("Char_Select_packet", "server_port", map_port);
    RFIFOSKIP(28);
    close_session();
  } else if(RFIFOW(0)==0x006c) {
    switch(RFIFOB(2)) {
      case 0:
        ok("Error", "Access denied");
        break;
      case 1:
        ok("Error", "Cannot use this ID");
        break;
    }
    state = CHAR_SELECT;
    RFIFOSKIP(3);
  }
  // Todo: add other packets
}

void server_char_delete() {
  state = CHAR_SELECT;
  // Delete a character
  if(!strcmp(button_state, "Del")) {
    if(yes_no("Confirm", "Are you sure?")==0) {
      // Request character deletion
      WFIFOW(0) = net_w_value(0x0068);
      WFIFOL(2) = net_l_value(char_info[0].id);
      WFIFOSET(46);

      while((in_size<2)||(out_size>0))flush();
      if(RFIFOW(0)==0x006f) {
        RFIFOSKIP(2);
        ok("Info", "Player deleted");
        free(char_info);
        n_character = 0;
      } else if(RFIFOW(0)==0x006c) {
        switch(RFIFOB(2)) {
          case 0:
            ok("Error", "Access denied");
            break;
          case 1:
            ok("Error", "Cannot use this ID");
            break;
        }
        RFIFOSKIP(3);
      } else ok("Error", "Unknown error");
    }
  // Create a new character
  } else {
    centre_dialog(char_create_dialog);
    DIALOG_PLAYER *player = init_dialog(char_create_dialog, -1);
    int gui_exit = 1;
    while ((!key[KEY_ESC])&&(gui_exit)) {
      clear_bitmap(buffer);
      blit((BITMAP *)graphic[LOGIN_BMP].dat, buffer, 0, 0, 0, 0, 800, 600);
      gui_exit = gui_update(player);
      blit(buffer, screen, 0, 0, 0, 0, 800, 600);
    }
    gui_exit = shutdown_dialog(player);
    if(gui_exit==3) {
      WFIFOW(0) = net_w_value(0x0067);
      strcpy(WFIFOP(2), name);
      WFIFOB(26) = net_b_value(5);
      WFIFOB(27) = net_b_value(5);
      WFIFOB(28) = net_b_value(5);
      WFIFOB(29) = net_b_value(5);
      WFIFOB(30) = net_b_value(5);
      WFIFOB(31) = net_b_value(5);
      WFIFOB(32) = net_b_value(0);
      WFIFOW(33) = net_w_value(1);
      WFIFOW(35) = net_w_value(1);
      WFIFOSET(37);

      while((in_size<3)||(out_size>0))flush();
      if(RFIFOW(0)==0x006d) {
        while(in_size<108)flush();
        char_info = (PLAYER_INFO *)malloc(sizeof(PLAYER_INFO));
        char_info->id = account_ID;
        memset(char_info->name, '\0', 24);
        strcpy(char_info[0].name, RFIFOP(2+74));
        char_info->hp = RFIFOW(2+42);
        char_info->max_hp = RFIFOW(2+44);
        char_info->sp = RFIFOW(2+46);
				char_info->max_sp = RFIFOW(2+48);
				char_info->job_lv = RFIFOL(2+16);
        char_info->job_xp = RFIFOL(2+12);
				char_info->lv = RFIFOW(2+58);
				char_info->xp = RFIFOL(2+4);
				char_info->gp = RFIFOL(2+8);
				char_info->STR = RFIFOB(2+98);
				char_info->AGI = RFIFOB(2+99);
				char_info->VIT = RFIFOB(2+100);
				char_info->INT = RFIFOB(2+101);
				char_info->DEX = RFIFOB(2+102);
				char_info->LUK = RFIFOB(2+103);
				RFIFOSKIP(108);
				n_character++;
			} else if(RFIFOW(0)==0x006c) {
				switch(RFIFOB(2)) {
          case 0:
            ok("Error", "Access denied");
            break;
          case 1:
            ok("Error", "Cannot use this ID");
            break;
        }
        RFIFOSKIP(3);
      } else ok("Error", "Unknown error");
    }
  }
}
