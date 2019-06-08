#include <stdio.h>
#include <string.h>
#include <BasicIntegerArithmetic.h>
#include <ctype.h>
#include <LinuxKernelSystemCall.h>
#include <LinuxKernelDefinition.h>

uLL count_graph(char *str)
{
	uLL i,ans;
	ans=0;
	for (i=0;str[i]>0;i++) {ans+=(isgraph(str[i])!=0);}
	return ans;
}
//remove trailing and leading space&return&newline&tab
int removeblank(char *str)
{
	long i,j,k;
	if (*str==0) {return 0;}
	for (i=0;(str[i]==' ')||(str[i]=='\n')||(str[i]=='\r')||(str[i]=='\t');i++);
	for (j=strlen(str)-1;j>=i;j--) {if ((str[j]!=' ')&&(str[j]!='\n')&&(str[j]!='\r')&&(str[i]!='\t')) {break;}}
	if (i<=j) {memcpy(str,&str[i],j-i+1);str[j-i+1]='\0';} else {str[0]='\0';}
	return 0;
}
//non-overlap:str,ans
//support %f,%e,%d
//assume str & name is safe
uLL build_placeholder_repalce(const char *str,const char *name,char *ans,uLL maxlen)
{
	int i,j,l,e1,e2,e3;
	ans[0]=0;
	for (i=0;name[i]>0;i++);e1=i;
	for (;i>0;i--) {if ((name[i]=='.')||(name[i]=='/')) {break;}}
	if (i<0) {e2=e1;e3=e1;}
	else
	{
		e2=(name[i]=='.')?i:e1;
		for (;i>0;i--) {if (name[i]=='/') {break;}}
		e3=(i<0)?e1:i;
	}
	for (i=0;str[i]>0;i++);
	maxlen-=i+1;
	if ((LL)maxlen<0) {goto OVERFLOW;}
	for (i=0,j=0;str[i]>0;i++)
	{
		if (str[i]=='\%')
		{
			i+=1;
			if (str[i]=='f') {
				maxlen-=e1;if ((LL)maxlen<0) {goto OVERFLOW;}
				for (l=0;l<e1;l++) {ans[j++]=name[l];} continue;
			}else if (str[i]=='e') {
				maxlen-=e2;if ((LL)maxlen<0) {goto OVERFLOW;}
				for (l=0;l<e2;l++) {ans[j++]=name[l];} continue;
			}else if (str[i]=='d') {
				maxlen-=e3;if ((LL)maxlen<0) {goto OVERFLOW;}
				for (l=0;l<e3;l++) {ans[j++]=name[l];} continue;
			}else if (str[i]==0) {break;}
		}
		ans[j++]=str[i];
	}
	ans[j++]=0;
	return j;
	OVERFLOW:
	ans[0]=0;
	return (uLL)0-1;
}
uLL config_exist(const char *path)
{
	uLL a,b,c;
	struct linux_file_stat t;
	a=stat(path,&t);
	if ((LL)a<0)
	{
		a=open(path,O_CREAT,00666);
		return a;
	}
	return 0;
}
uLL close_config(const uLL fd)
{
	uLL a;
	a=close(fd);
	return a;
}
#ifndef NOMAIN
int main()
{
	static char *str1="gcc -O3 \"%f\" -static -fno-builtin -fno-stack-protector -fno-stack-limit -march=native -mcmodel=large -I \"$HOME/FDTH/CCE/Headers\" -I \"$HOME/FDTH/CCE/EHeaders\" -L \"$HOME/FDTH/CCE/Library\" -L \"$HOME/FDTH/CCE/ELibrary\" -lm -o \"%e\" -w";
	static char *str2="/usr/local/share/buck-?q.cpp";
	static char ans[100000];
	int a;
	a=build_placeholder_repalce(str1,str2,ans,512);
	printf("%d\n",a);
	printf("%s\n",ans);
	printf("%d\n",count_graph(ans));
	return 0;
}
#endif