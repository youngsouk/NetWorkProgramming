#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>

char key = 34;

int get_f_size(unsigned char * obj){
	unsigned char * tmp;
	int i;

	for(i = 0;;i++){
		if(*(obj+i) == 0xc3)
			break;
	}


	return i;
}

unsigned char * encrypt(unsigned char * origin, int f_size){
	size_t pagesize = sysconf(_SC_PAGESIZE);
	if(pagesize > f_size ) f_size = pagesize;
	unsigned char * res = malloc(f_size);
	// mprotect(res, f_size, 7);

	for(int i = 0; i < f_size; i++){
		*(res + i) = *(origin + i) ^ key;
		// printf("0x%02x", *res);
	}
	return res;
}

void * decrypt(unsigned char * obj, int f_size){
	for(int i = 0; i < f_size; i++){
		*(obj + i)= *(obj + i) ^ key;
	}

}

void o_function(){
	printf("\nddfsdfdsfsd\n");
}

void p_f_code(unsigned char * function, int size){
	printf("[");
	for(int i = 0; i < size; i++){
		printf("0x%02x, ", *(function + i));
	}
	printf("]\n");
	
}
int main(){
	unsigned char * obj = (unsigned char*)o_function;
	void (*f)();
	int f_size;

	printf("PID : %d\n", getpid());
	f_size = get_f_size(obj);
	printf("function size : %d\n", f_size);
	f = encrypt(obj, f_size);
	p_f_code((unsigned char*)f, f_size);
	decrypt((unsigned char*)f, f_size);
	p_f_code((unsigned char*)f, f_size);
	(*f)();


}