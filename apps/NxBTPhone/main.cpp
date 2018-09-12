//#include "MainDialog.h"
#include <QApplication>
#include <QTranslator>
#include <getopt.h>	// for getopt_long()
#include "defines.h"

#ifndef BUFFER_SIZE
#	define BUFFER_SIZE	1024
#endif

#ifndef MIN
#define MIN(A, B) A > B ? B : A
#endif

Menu g_current_menu, g_previous_menu;
bool g_calling_end_is_exit;
#ifdef CONFIG_TEST_FLAG
bool g_first_shown = false;
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	// default menu is select menu.
	g_current_menu = Menu_Select;
	g_calling_end_is_exit = false;

	static struct option option_table[] =
	{
		{ .name = "help",   .has_arg = no_argument,         .flag = 0,  .val = 'h' },
		{ .name = "menu",   .has_arg = required_argument,   .flag = 0,  .val = 'm' },
		{ .name = 0,        .has_arg = 0,                   .flag = 0,  .val = 0   }
	};

	int32_t option = 0;
	// getopt_long stores the option index here
	int32_t index = 0;
	char buffer[BUFFER_SIZE] = {0,};

	while ( ( option = getopt_long(argc, argv, "m:h", option_table, &index) ) != -1)
	{
		switch (option)
		{
		case 'm':
			memcpy( buffer, optarg, MIN( strlen(optarg), BUFFER_SIZE ) );
			printf("menu options : %s\n", buffer);

			if ( strcmp( buffer, "select" ) == 0 ) {
				g_current_menu = Menu_Select;
			} else if ( strcmp( buffer, "call" ) == 0 ) {
				g_current_menu = Menu_Call;
			} else if ( strcmp( buffer, "message" ) == 0 ) {
				g_current_menu = Menu_Message;
			} else if ( strcmp (buffer, "calling" ) == 0 ) {
				g_current_menu = Menu_Calling;
				g_calling_end_is_exit = true;
			}

			break;

		case 'h':
			break;

		case '?':
			break;

		default:
			break;
		}
	}

//    MainDialog w;

    return a.exec();
}
