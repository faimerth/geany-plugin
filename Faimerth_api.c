//#define DEBUG
#ifdef DEBUG
#include <MacroAutomaton.h>
// is fatal?,assum x is true, message...
#define assert(fatal,x,...)\
({	if ((x)==0)\
	{\
		write(2,("Assert Error: " __VA_ARGS__ "\n" __FILE__ ":" STR_MAC(__LINE__) ": '" #x "'\n")\
		,sizeof(("Assert Error: " __VA_ARGS__ "\n" __FILE__ ":" STR_MAC(__LINE__) ": '" #x "'\n")));\
		if ((fatal)) {exit_group((uLL)0-1);};\
	};((x)!=0);})
#else
#define assert(...) ({(1);})
#endif

#ifdef HAVE_CONFIG_H
	#include "config.h" /* for the gettext domain */
#endif

#include <string.h>
#ifdef HAVE_LOCALE_H
	#include <locale.h>
#endif

#define uLL unsigned long long
#define LL long long
#include <gdk/gdkkeysyms.h>
#include <geanyplugin.h>
#include <geany.h>
#include <document.h>

#include "Scintilla.h"
#include "SciLexer.h"

#define SSM(s, m, w, l) scintilla_send_message(s, m, w, l)
#define MAXWORDLENGTH 32

GeanyPlugin	*geany_plugin;
GeanyData	*geany_data;

static gint source_id;

static const gint HL_INDICATOR = GEANY_INDICATOR_SEARCH;

////////////////////////
//string function
//count meaningfull characters
uLL count_graph(char *str);

//remove trailing and leading space&return&newline&tab
int removeblank(char* str);

//non-overlap:str,ans
//support %f,%e,%d
//assume str & name is safe
uLL build_placeholder_repalce(const char *str,const char *name,char *ans,uLL maxlen);
//////////////////////////////

//-----------------config----------------
#include "Faimerth_config.h"
//////////////////////////////////////////

static void search_mark_in_range(
	GeanyEditor *editor,
	gint         flags,
	struct       Sci_TextToFind *ttf)
{
	ScintillaObject *sci = editor->sci;

	while (SSM(sci, SCI_FINDTEXT, flags, (uptr_t)ttf) != -1)
	{
		gint start = ttf->chrgText.cpMin;
		gint end = ttf->chrgText.cpMax;

		if (end > ttf->chrg.cpMax)
			break;

		ttf->chrg.cpMin = ttf->chrgText.cpMax;
		if (end == start)
			continue;
		if(SSM(sci, SCI_INDICATORVALUEAT, HL_INDICATOR, start))
			continue;
		SSM(sci, SCI_SETINDICATORCURRENT, HL_INDICATOR, 0);
		SSM(sci, SCI_INDICATORFILLRANGE, start, end - start);
	}
}

static gboolean autoHighLight(gpointer user_data)
{
	GeanyDocument      *doc = (GeanyDocument *)user_data;
	GeanyEditor        *editor = doc->editor;
	static GeanyEditor *editor_cache = NULL;
	ScintillaObject    *sci = editor->sci;
	gchar              *text=NULL;
	static gchar        text_cache[MAXWORDLENGTH] = {0};
	gint                match_flag = SCFIND_MATCHCASE | SCFIND_WHOLEWORD;
	struct              Sci_TextToFind ttf;

	source_id = 0;

	/* during timeout document could be destroyed so check everything again */
	if (!DOC_VALID(doc)) {return FALSE;}

	if (sci_has_selection(sci))
	{
		if (sci_get_selected_text_length(sci)>MAXWORDLENGTH) {return FALSE;};
		text=sci_get_selection_contents(sci);
		if (editor_cache != editor || strcmp(text, text_cache) != 0)
		{
			editor_indicator_clear(editor, HL_INDICATOR);
			strcpy(text_cache, text);
			editor_cache = editor;
		}
		gint vis_first = SSM(sci, SCI_GETFIRSTVISIBLELINE, 0, 0);
		gint doc_first = SSM(sci, SCI_DOCLINEFROMVISIBLE, vis_first, 0);
		gint vis_last  = SSM(sci, SCI_LINESONSCREEN, 0, 0) + vis_first;
		gint doc_last  = SSM(sci, SCI_DOCLINEFROMVISIBLE, vis_last, 0);
		gint start     = SSM(sci, SCI_POSITIONFROMLINE,   doc_first, 0);
		gint end       = SSM(sci, SCI_GETLINEENDPOSITION, doc_last, 0);

		ttf.lpstrText  = text;
		ttf.chrg.cpMin = start;
		ttf.chrg.cpMax = sci_get_selection_start(sci);
		search_mark_in_range(editor, match_flag, &ttf);
		//printf("%llu\n",(uLL)editor_show_calltip);
		ttf.chrg.cpMin = sci_get_selection_end(sci);
		ttf.chrg.cpMax = end;
		search_mark_in_range(editor, match_flag, &ttf);

		g_free(text);
		return FALSE;
	}
	else
	{
		editor_indicator_clear(editor, HL_INDICATOR);
		return FALSE;
	}

	return FALSE;
}
GeanyLexerStyle caret_styling={0x000000,0x000000,0,0};//foreground(int),background(int),bold(t/f),italic(t/f)
ScintillaObject *csci=NULL;
char CapsStatus()
{
	static int fd;
	char ch;
	if (fd==0)
	{
		fd=open("/sys/class/leds/input4\:\:capslock/brightness",0);
	}
	lseek(fd,0,0);
	read(fd,&ch,1);
	if (config_dirty&((uLL)1<<GENERAL_CONFIG)) {load_config(GENERAL_CONFIG);config_dirty-=((uLL)1<<GENERAL_CONFIG);}
	ch=(ch=='1');
	caret_styling.foreground=mode_switch[ch][0];
	caret_styling.background=mode_switch[ch][1];
	caret_styling.bold=mode_switch[ch][2];
	caret_styling.italic=mode_switch[ch][3];
	return ch;
}
void CapsSwitch(guint key_id)
{
	CapsStatus();
	if (csci!=NULL)
	{
		SSM(csci, SCI_SETCARETFORE, caret_styling.foreground, 0);
		SSM(csci, SCI_SETCARETWIDTH, caret_styling.background , 0);
	}
}
static gboolean on_editor_notify(GObject *obj,GeanyEditor *editor,SCNotification *nt,gpointer user_data)
{
	if (nt->nmhdr.code==SCN_FOCUSIN)
	{
		CapsStatus();
		csci=(GeanyDocument *)(editor->document)->editor->sci;
		if (csci!=NULL)
		{
			SSM(csci, SCI_SETCARETFORE, caret_styling.foreground, 0);
			SSM(csci, SCI_SETCARETWIDTH, caret_styling.background , 0);
		}
	}
	if (nt->nmhdr.code==SCN_UPDATEUI)
	{
		/* if events are too intensive - remove old callback */
		if (source_id) {g_source_remove(source_id);}
		source_id = g_idle_add(autoHighLight, editor->document);
	}
	return FALSE;
}
static gboolean on_document_activate(GObject *obj,GeanyDocument *doc,gpointer user_data)
{
	if (doc->editor)
	{
		csci=doc->editor->sci;
		SSM(csci, SCI_SETCARETFORE, caret_styling.foreground, 0);
		SSM(csci, SCI_SETCARETWIDTH, caret_styling.background , 0);
	}
	return FALSE;
}
static gboolean on_document_open(GObject *obj,GeanyDocument *doc,gpointer user_data)
{
	if (doc->editor)
	{
		csci=doc->editor->sci;
		SSM(csci, SCI_SETCARETFORE, caret_styling.foreground, 0);
		SSM(csci, SCI_SETCARETWIDTH, caret_styling.background , 0);
	}
	return FALSE;
}

static gboolean on_document_save(GObject *obj, GeanyDocument *doc, gpointer user_data)
{
	LL a;
	if (config_op_num>0)
	{
		a=get_config_id_from_doc(doc);
		if (a>=0) {config_dirty|=(uLL)1<<a;}
	}
	return FALSE;
}

static gboolean on_document_reload(GObject *obj, GeanyDocument *doc, gpointer user_data)
{
	LL a;
#ifdef DEBUG
	printf("doc_obj %llx\n",(uLL)doc);
#endif
	if (config_op_num>0)
	{
		a=get_config_id_from_doc(doc);
		if (a>=0) {config_dirty|=(uLL)1<<a;}
	}
	return FALSE;
}

static gboolean on_document_close(GObject *obj, GeanyDocument *doc, gpointer user_data)
{
	LL a;
	csci=NULL;
	if (config_op_num>0)
	{
		a=get_config_id_from_doc(doc);
		if (a>=0)
		{
			config_op_num-=1;
			config_stat[a].doc_obj=0;
		}
	}
	return FALSE;
}

int get_id(const *str)
{
	long i;
	for (i=2;i<ftsize;i++)
	{
		if (strcmp(str,fty[i].name)==0) {return i;}
	}
	return 0;
}
void cmd_finished(GPid child_pid,gint status,gpointer data)
{
	ui_progress_bar_stop();
	if (SPAWN_WIFEXITED(status))
	{
		msgwin_msg_add(COLOR_BLUE,-1,NULL,"Command finished successfully with %d returned.",SPAWN_WEXITSTATUS(status));
	}
	else
	{
		msgwin_msg_add(COLOR_BLUE,-1,NULL,"Command terminated unexpectly.");
	}
}
void cmd_read_io(GString *string,GIOCondition condition,gpointer data)
{
	if (condition&(G_IO_IN|G_IO_PRI))
	{
		removeblank(string->str);
		msgwin_msg_add(COLOR_BLACK,-1,NULL,"%s",string->str);
	}
}
gboolean RunCmd(GeanyKeyBinding *key, guint key_id, gpointer user_data)
{
#ifdef DEBUG
	printf("RunCmd: key_id %d\n",key_id);
#endif
	GeanyDocument *doc;
	int x,y;
	char tmp[256],tmp2[256],*s1,*s2,*argv[3];
	long i,j,k,l;
	doc=document_get_current();
	y=(uLL)user_data;
#ifdef DEBUG
	printf("RunCmd: y:%llu\n",(uLL)user_data);
#endif
	if (config_dirty&((uLL)1<<CMD_CONFIG)) {load_config(CMD_CONFIG);config_dirty-=((uLL)1<<CMD_CONFIG);}
	if (y>=max_cmd) {y=y-max_cmd;x=0;}
	else
	{
		x=get_id(doc->file_type->name);
		s1=doc->file_name;
		if (x<2) {x=1;}
	}
	if (!((x>=0)&&(x<ftsize)&&(y>=0)&&(y<max_cmd)))
	{
		printf("ERROR: RunCmd x,y %d,%d\n",x,y);
		return FALSE;
	}
	if (fty[x].cmd[y].ex!=NULL)
	{
		if (fty[x].cmd[y].pa!=NULL)
		{
			l=build_placeholder_repalce(fty[x].cmd[y].pa,doc->file_name,tmp,256);
			if (l<0) {printf("ERROR: Faimerth Plugin: RunCmd: build_placeholder_repalce\n");}
#ifdef DEBUG
			printf("pa:%s -%s-\n",fty[x].cmd[y].pa,tmp);
#endif
			argv[0]=&tmp;argv[1]=NULL;
		}
		else
		{
			*tmp=0;
			argv[0]=NULL;
		}
		msgwin_clear_tab(MSG_MESSAGE);
		msgwin_status_add("Command: -%s- \"%s\"",fty[x].cmd[y].ex,tmp);
		msgwin_msg_add(COLOR_BLUE,-1,NULL,"Command: -%s- \"%s\"",fty[x].cmd[y].ex,tmp);
		if (fty[x].cmd[y].wd>0) {if (count_graph(fty[x].cmd[y].wd)>0)
		{
			l=build_placeholder_repalce(fty[x].cmd[y].wd,doc->file_name,tmp2,256);
		}
		else
		{
			l=build_placeholder_repalce("%d",doc->file_name,tmp2,256);
		}}
		if (l<0) {printf("ERROR: Faimerth Plugin: RunCmd: build_placeholder_repalce\n");}
		msgwin_msg_add(COLOR_BLUE,-1,NULL,"Working Directory: -%s-",tmp2);
		if (spawn_with_callbacks(tmp2,fty[x].cmd[y].ex,argv,NULL,SPAWN_STDOUT_RECURSIVE,NULL,NULL,
		 cmd_read_io,NULL,0,cmd_read_io,NULL,0,cmd_finished,NULL,NULL,NULL))
		{
			ui_progress_bar_start("Running...");
		}
		else
		{
			msgwin_msg_add(COLOR_BLUE,-1,NULL,"Command failed !!!\n");
			ui_set_statusbar(TRUE,"Run command failed !!!");
		}
		return TRUE;
	}
	return FALSE;
}

static GtkWidget *main_menu_item = NULL;
PluginCallback plugin_callbacks[] =
{
	{ "editor-notify",(GCallback)&on_editor_notify,FALSE,NULL },
	{ "document-activate",(GCallback)&on_document_activate,FALSE,NULL},
	{ "document-close",(GCallback)&on_document_close,FALSE,NULL},
	{ "document-open",(GCallback)&on_document_open,FALSE,NULL},
	{ "document-save",(GCallback)&on_document_save,FALSE,NULL},
	{ "document-reload",(GCallback)&on_document_reload,FALSE,NULL},
	{ NULL, NULL, FALSE, NULL }
};
void plugin_cleanup(void)
{
	int i,j;
	//autohilght
	if (source_id) {g_source_remove(source_id);}
	/* remove the menu item added in demo_init() */
	gtk_widget_destroy(main_menu_item);
	/* release other allocated strings and objects */
	delete_config();
}

void plugin_help(void)
{
}

void item_activate_reload(GtkMenuItem *menuitem, gpointer gdata)
{
	LL i,a;
	for (i=0;i<config_size;i++)
	{
		a=load_config(i);
		if (a>=0) {msgwin_status_add(_("Config File: %s reloaded."),config_stat[i].file_name);}
	}
}
void item_activate_op(GtkMenuItem *menuitem, uLL gdata)
{
	if (assert(0,(gdata<config_size)&&(gdata>=0),"open config file"))
	{
		if (assert(0,config_stat[gdata].file_name>0))
		{
			if (config_stat[gdata].doc_obj==0)
			{
				config_op_num+=1;
				config_stat[gdata].doc_obj=document_open_file(config_stat[gdata].file_name,FALSE,NULL,"UTF-8");
#ifdef DEBUG
	printf("doc_obj %llx\n",(uLL)config_stat[gdata].doc_obj);
#endif
			}
		}
	}
}

#include "Faimerth_gui.h"
void plugin_init(GeanyData *data)
{
	LL i,j,k;
	FILE *temp;
	//------initailization--------

	//key binding
	GeanyKeyGroup *key_group,*key_group_s,*key_group_u;

	key_group = plugin_set_key_group(geany_plugin, "faimerth_general",10, NULL);
	keybindings_set_item(key_group,0,CapsSwitch,0, 0, "Insert Mode ON", _("Insert Mode ON"), NULL);
	keybindings_set_item(key_group,1,CapsSwitch,0, 0, "Insert Mode OFF", _("Insert Mode OFF"), NULL);

	//key_group_s = plugin_set_key_group(geany_plugin, "faimerth_cus_cmd",4, NULL);
	keybindings_set_item_full(key_group,2,0,0,"Run 0",_("Run 0"),NULL,RunCmd,0,NULL);
	keybindings_set_item_full(key_group,3,0,0,"Run 1",_("Run 1"),NULL,RunCmd,1,NULL);
	keybindings_set_item_full(key_group,4,0,0,"Run 2",_("Run 2"),NULL,RunCmd,2,NULL);
	keybindings_set_item_full(key_group,5,0,0,"Run 3",_("Run 3"),NULL,RunCmd,3,NULL);


	//key_group_u = plugin_set_key_group(geany_plugin, "faimerth_uni_cmd",4, NULL);
	keybindings_set_item_full(key_group,6,0,0,"exec 0",_("exec 0"),NULL,RunCmd,4,NULL);
	keybindings_set_item_full(key_group,7,0,0,"exec 1",_("exec 1"),NULL,RunCmd,5,NULL);
	keybindings_set_item_full(key_group,8,0,0,"exec 2",_("exec 2"),NULL,RunCmd,6,NULL);
	keybindings_set_item_full(key_group,9,0,0,"exec 3",_("exec 3"),NULL,RunCmd,7,NULL);

	//config_file
	config_stat[GENERAL_CONFIG].file_name = g_strconcat(geany->app->configdir, G_DIR_SEPARATOR_S, "plugins",
		G_DIR_SEPARATOR_S, "faimerth", G_DIR_SEPARATOR_S, "general.conf", NULL);
	config_stat[CMD_CONFIG].file_name = g_strconcat(geany->app->configdir, G_DIR_SEPARATOR_S, "plugins",
		G_DIR_SEPARATOR_S, "faimerth", G_DIR_SEPARATOR_S, "custom-command.conf", NULL);
	for (i=0;i<config_size;i++) {load_config(i);}

	//autohilight
	source_id = 0;

	//mode switch
	caret_styling.foreground=0x000000;caret_styling.background=0x000002;

	//------UI-------
	main_menu_ini();
}
void geany_load_module(GeanyPlugin *plugin)
{
	geany_plugin=plugin;
	geany_data=plugin->geany_data;

	plugin->info->name = _("Faimerth");
	plugin->info->description = _("1.Autohighlight your selected word.\n2.Capslock mode switch indicate.\n3.Run command.");
	plugin->info->version = "0.4";
	plugin->info->author =  _("Faimerth");

	plugin->funcs->init = plugin_init;
	plugin->funcs->configure = configure_diag_activate;
	plugin->funcs->help = plugin_help; /* This demo has no help but it is an option */
	plugin->funcs->cleanup = plugin_cleanup;
	plugin->funcs->callbacks = plugin_callbacks;

	GEANY_PLUGIN_REGISTER(plugin, 225);
}
