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

//check file exist.
//if not create one with mode 00666 and return file discreptor.
uLL config_exist(const char *path);

//wrapper for close system call
uLL close_config(const uLL fd);

//wrapper for write system call
LL write_flush(const uLL fd,const char *buf,const uLL size);
LL write_buf2(const uLL fd,char *buf,uLL size,const uLL x,const uLL limit);
LL write_buf(const uLL fd,char *buf,uLL size,const char *str,const uLL limit);
//////////////////////////////