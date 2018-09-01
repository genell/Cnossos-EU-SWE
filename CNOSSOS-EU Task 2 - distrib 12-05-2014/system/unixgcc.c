#include <string.h>
#include <ctype.h>

/**
 * gcc does not have strcmpi implementation 
 */
int _strcmpi(const char* s1, const char* s2){
	int i;
	
	if(strlen(s1)!=strlen(s2))
		return -1;
		
	for(i=0;i<strlen(s1);i++){
		if(toupper(s1[i])!=toupper(s2[i]))
			return s1[i]-s2[i];
	}
	return 0;
}
