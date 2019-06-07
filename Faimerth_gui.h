/* Callback when the menu item is clicked. */
void item_activate(GtkMenuItem *menuitem, gpointer gdata)
{
	GtkWidget *dialog;
	GeanyPlugin *plugin = gdata;
	GeanyData *geany_data = plugin->geany_data;

	dialog = gtk_message_dialog_new(
		GTK_WINDOW(geany_data->main_widgets->window),
		GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_INFO,
		GTK_BUTTONS_OK,
		"%s", "fuck you");
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),
		_("(From the %s plugin)"), plugin->info->name);

	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}
/* Callback connected in demo_configure(). */
static void
on_configure_response(GtkDialog *dialog, gint response, gpointer user_data)
{
	/* catch OK or Apply clicked */
	if (response == GTK_RESPONSE_OK || response == GTK_RESPONSE_APPLY)
	{
		/* We only have one pref here, but for more you would use a struct for user_data */
		GtkWidget *entry = GTK_WIDGET(user_data);

		//g_free(welcome_text);
		//welcome_text = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
		/* maybe the plugin should write here the settings into a file
		 * (e.g. using GLib's GKeyFile API)
		 * all plugin specific files should be created in:
		 * geany->app->configdir G_DIR_SEPARATOR_S plugins G_DIR_SEPARATOR_S pluginname G_DIR_SEPARATOR_S
		 * e.g. this could be: ~/.config/geany/plugins/Demo/, please use geany->app->configdir */
	}
}

/* Called by Geany to show the plugin's configure dialog. This function is always called after
 * demo_init() was called.
 * You can omit this function if the plugin doesn't need to be configured.
 * Note: parent is the parent window which can be used as the transient window for the created
 *       dialog. */
static GtkWidget *configure_diag_activate(GeanyPlugin *plugin, GtkDialog *dialog, gpointer data)
{
	GtkWidget *label, *entry, *vbox, *notebook, *notebook_vbox, *notebook_hbox;
	/* example configuration dialog */
	vbox = gtk_vbox_new(FALSE, 1);

	notebook = gtk_notebook_new();
	gtk_widget_set_can_focus(notebook, FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(notebook), 5);
	gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);

	{
		notebook_vbox = gtk_vbox_new(FALSE, 2);
		gtk_notebook_insert_page(GTK_NOTEBOOK(notebook),notebook_vbox,gtk_label_new("General"),0);

		//notebook_hbox = gtk_hbox_new(
		/* add a label and a text entry to the dialog */
		label = gtk_label_new(_("Welcome text to show:"));
		gtk_label_set_xalign(label,0);
		gtk_label_set_yalign(label,0.5);
		entry = gtk_entry_new();

		gtk_entry_set_text(GTK_ENTRY(entry), "eat shit");

		gtk_container_add(GTK_CONTAINER(notebook_vbox), label);
		gtk_container_add(GTK_CONTAINER(notebook_vbox), entry);
	}

	gtk_widget_show_all(vbox);

	/* Connect a callback for when the user clicks a dialog button */
	g_signal_connect(dialog, "response", G_CALLBACK(on_configure_response), entry);
	return vbox;
}
void main_menu_ini()
{
	//---------UI------------
	GtkWidget *main_item,*main_item_menu,*item[4];
	/* Add an item to the Tools menu */
	main_item = gtk_menu_item_new_with_mnemonic(_("_Faimerth Plugin"));
	gtk_container_add(GTK_CONTAINER(geany->main_widgets->tools_menu), main_item);
	main_item_menu = gtk_menu_new ();
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(main_item), main_item_menu);
	/* 0 */
	item[0] = gtk_menu_item_new_with_mnemonic(_("Reload Config"));
	gtk_container_add(GTK_CONTAINER (main_item_menu), item[0]);
	g_signal_connect(item[0], "activate", G_CALLBACK(item_activate_reload), NULL);
	/* 1 */
	item[1] = gtk_menu_item_new_with_mnemonic(_("Open General Config file"));
	gtk_container_add(GTK_CONTAINER (main_item_menu), item[1]);
	g_signal_connect(item[1], "activate", G_CALLBACK(item_activate_op), GENERAL_CONFIG);

	/* 2 */
	item[1] = gtk_menu_item_new_with_mnemonic(_("Open Command Config file"));
	gtk_container_add(GTK_CONTAINER (main_item_menu), item[1]);
	g_signal_connect(item[1], "activate", G_CALLBACK(item_activate_op), CMD_CONFIG);


	/* make the menu item sensitive only when documents are open */
	ui_add_document_sensitive(main_item);
	/* keep a pointer to the menu item, so we can remove it when the plugin is unloaded */
	main_menu_item = main_item;

	gtk_widget_show_all(main_item);
}