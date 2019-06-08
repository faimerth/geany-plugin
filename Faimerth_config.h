static struct string{
	LL size;
	char *s;
};
#define sstring(str) {sizeof(str),str}

//----general----------
static struct config_item{
	struct string name;
	char type;//0 int, 1 string
	union{
		uLL num;
		char *str;
	}ini;
	union{
		uLL *num;
		char **str;
	}value;
};
static struct config_dir{
	struct string name;
	uLL item_size;
	struct config_item *arr;
	struct string help;
};
static uLL mode_switch[2][4]={{0,2,0,0},{0x2A2AA5,3,0,0}};//mode switch 'caret_styling'
static struct config_item mode_switch_config[8]={
	{sstring("clr_on_0"),0,{0x2A2AA5},{&mode_switch[1][0]}},
	{sstring("clr_on_1"),0,{3},{&mode_switch[1][1]}},
	{sstring("clr_on_2"),0,{0},{&mode_switch[1][2]}},
	{sstring("clr_on_3"),0,{0},{&mode_switch[1][3]}},
	{sstring("clr_off_0"),0,{0x000000},{&mode_switch[0][0]}},
	{sstring("clr_off_1"),0,{2},{&mode_switch[0][1]}},
	{sstring("clr_off_2"),0,{0},{&mode_switch[0][2]}},
	{sstring("clr_off_3"),0,{0},{&mode_switch[0][3]}},
};
static uLL config_dir_size=1;
static struct config_dir config_dir[]={
	{sstring("mode"),8,mode_switch_config,sstring("#foreground,background,bold,italic\n")},
};

//----CMD----------
static const int ftsize=4,max_cmd=4;//type size, command size
static struct {
	char csize;
	union{
		struct{
			char *label,*ex,*pa,*wd;
		}cmd[4];
		char *exe[4][4];
	};
	struct string name,loc;
}fty[]={
	{0,{{{0,},}},sstring("Universal"),{0,0}},
	{0,{{{0,},}},sstring("Default"),{0,0}},
	{0,{{{0,},}},sstring("C"),sstring(".c")},
	{0,{{{0,},}},sstring("C++"),sstring(".cpp")}};
static struct string fty_d[]={
	sstring("#External program executed for exec shortcuts\n#%e -- no extension\n#%f -- full path\n#%d -- directory\n"),
	sstring("#Action for unkown filetype\n"),
	sstring("#Action for C files\n"),
	sstring("#Action for C++ files\n"),
};
static const uLL fey[]={4/*number of entries*/,'L'+'B'*0x100,'E'+'X'*0x100,'P'+'A'*0x100,'W'+'D'*0x100};
static int pid=-1;
/////////////////////////////////
#define GENERAL_CONFIG (0)
#define CMD_CONFIG (1)
static uLL config_size=2;
static uLL config_dirty=0,config_op_num=0;
static struct{
	char *name,*file_name;
	GeanyDocument *doc_obj;
}config_stat[4]={{"general",0,0},{"cmd",0,0}};//0 general, 1 cmd
//---config function
LL get_config_id_from_doc(void *p)
{
	LL i,a,b;
	a=(LL)p;
	for (i=0;i<config_size;i++)
	{
		b=(LL)config_stat[i].doc_obj;
		if (a==b) {return i;}
	}
	return (LL)0-1;
}
LL get_config_id_from_name(const char *str);
LL load_cmd_config()
{
	char *cmd_config_file=config_stat[CMD_CONFIG].file_name;
	uLL i,j,k,l;
	if (cmd_config_file>0)
	{
		GKeyFile *config = g_key_file_new();
	#ifdef DEBUG
		printf("plugin load config from <%s>\n",cmd_config_file);
	#endif
		g_key_file_load_from_file(config, cmd_config_file, G_KEY_FILE_NONE, NULL);
		for (i=0;i<ftsize;i++)
		{
			for (l=0;l<max_cmd;l++)
			{
				for (j=0;j<fey[0];j++)
				{
					k=fey[j+1]+((uLL)'_'<<16)+((uLL)'0'<<24)+((l+'0')<<32);
	#ifdef DEBUG
		printf("load <%s> cmd %d <%s> [%s]::\n",fty[i].name,l,(char*)&fey[j+1],(char*)&k);
	#endif
					if (fty[i].exe[l][j]>0) {g_free(fty[i].exe[l][j]);}
					fty[i].exe[l][j] = utils_get_setting_string(config, fty[i].name.s, (gchar*)&k, NULL);
	#ifdef DEBUG
		printf("\t%s\n",fty[i].exe[l][j]);
	#endif
				}
				if (fty[i].cmd[l].label==0) {break;}
			}
		}

		g_key_file_free(config);
		return 0;
	}
	else
	{
		return (LL)0-1;
	}
}
LL load_general_config()
{
	char *config_file=config_stat[GENERAL_CONFIG].file_name;
	LL i,j,k,l;
	struct config_item *items;
	if (assert(0,config_file>0,"general_config_file not present"))
	{
#ifdef DEBUG
	printf("plugin load config from <%s>\n",config_file);
#endif
		GKeyFile *config = g_key_file_new();
		g_key_file_load_from_file(config, config_file, G_KEY_FILE_NONE, NULL);
		for (i=0;i<config_dir_size;i++)
		{
			l=config_dir[i].item_size;
			items=config_dir[i].arr;
			for (j=0;j<l;j++)
			{
				if (items[j].type==0)
				{
					*(items[j].value.num)=utils_get_setting_integer(config,config_dir[i].name.s,items[j].name.s,items[j].ini.num);
#ifdef DEBUG
	printf("load %s : %s ::\n\t%lld\n",config_dir[i].name.s,items[j].name.s,*(items[j].value.num));
#endif
				}
				else
				{
					*(items[j].value.str)=utils_get_setting_string(config,config_dir[i].name.s,items[j].name.s,items[j].ini.str);
#ifdef DEBUG
	printf("load %s : %s ::\n\t%s\n",config_dir[i].name.s,items[j].name.s,*(items[j].value.num));
#endif
				}
			}
		}
		g_key_file_free(config);
		return 0;
	}
	else
	{
		return (LL)0-1;
	}
}
LL load_config(LL id)
{
	LL a,b,c;
	if (id==GENERAL_CONFIG)
	{
		a=load_general_config();
		return a;
	}
	else if (id==CMD_CONFIG)
	{
		a=load_cmd_config();
		if (a<0) {printf("ERROR: CMD config load error: config_stat[id:%lld].file_name is nil.\n",id);}
		return a;
	}
	else
	{
		printf("ERROR: function: load_config: config_stat[id:%lld] doesn't exit.\n",id);
		return (LL)0-1;
	}
}
LL delete_config()
{
	LL i,j,k,l;
	struct config_item *items;
	for (i=0;i<config_size;i++)
	{
		if (config_stat[i].file_name>0)
		{
			g_free(config_stat[i].file_name);
			config_stat[i].file_name=0;
		}
	}
	//general config
	for (i=0;i<config_dir_size;i++)
	{
		l=config_dir[i].item_size;
		items=config_dir[i].arr;
		for (j=0;j<l;j++)
		{
			if ((items[j].type==1)&&(*(items[j].value.str)>0))
			{
				g_free(*(items[j].value.str));
				*(items[j].value.str)=0;
			}
		}
	}
	//cmd config
	for (i=0;i<ftsize;i++)
	{
		for (j=0;j<max_cmd;j++)
		{
			for (l=0;l<fey[0];l++)
			{
				if (fty[i].exe[j][l]>0)
				{
					g_free(fty[i].exe[j][l]);
					fty[i].exe[j][l]=0;
				}
			}
		}
	}
	return 0;
}

uLL generate_empty_cmd_config(uLL fd)
{
	uLL i,j,k,l,size,p;
	char buf[4096];size=0;p=0;
	buf[p++]='#';
	p=write_buf2(fd,buf,p,ftsize-2,4096);
	p=write_buf(fd,buf,p," filetypes, ",4096);
	p=write_buf2(fd,buf,p,max_cmd,4096);
	p=write_buf(fd,buf,p," command support\n\n",4096);
	for (i=0;i<ftsize;i++)
	{
		p=write_buf(fd,buf,p,"[",4096);
		p=write_buf(fd,buf,p,fty[i].name.s,4096);
		p=write_buf(fd,buf,p,"]\n",4096);
		p=write_buf(fd,buf,p,fty_d[i].s,4096);
		for (j=0;j<fey[0];j++)
		{
			k=fey[j+1]+((uLL)'_'<<16)+((uLL)'0'<<24)+((l+'0')<<32);
			p=write_buf(fd,buf,p,(char*)&k,4096);
			p=write_buf(fd,buf,p,"=\n",4096);
		}
		p=write_buf(fd,buf,p,"\n",4096);
	}
	write_flush(fd,buf,p);
	close_config(fd);
}
uLL generate_empty_general_config(uLL fd)
{
	LL i,j,k,l,size,p;
	char buf[4096];size=0;p=0;
	struct config_item *items;
	for (i=0;i<config_dir_size;i++)
	{
		p=write_buf(fd,buf,p,"[",4096);
		p=write_buf(fd,buf,p,config_dir[i].name.s,4096);
		p=write_buf(fd,buf,p,"]\n",4096);
		p=write_buf(fd,buf,p,config_dir[i].help.s,4096);
		l=config_dir[i].item_size;
		items=config_dir[i].arr;
		for (j=0;j<l;j++)
		{
			p=write_buf(fd,buf,p,items[j].name.s,4096);
			p=write_buf(fd,buf,p,"=",4096);
			if (items[j].type==0)
			{
				p=write_buf2(fd,buf,p,items[j].ini.num,4096);
			}
			else if (items[j].type==1)
			{
				p=write_buf(fd,buf,p,items[j].ini.str,4096);
			}
			else {;}
			p=write_buf(fd,buf,p,"\n",4096);
		}
		p=write_buf(fd,buf,p,"\n",4096);
	}
	write_flush(fd,buf,p);
	close_config(fd);
	return 0;
}
