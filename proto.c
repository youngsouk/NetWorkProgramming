
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUF_SIZE        1024    // buf size

// msg header define
typedef struct {
        int id;
        char name[30];
        int age;
        char email[50];
}HEAD;

// integer to ascii
void itoa(int i, char *st)
{
        sprintf(st, "%d", i);
        return;
}

// string test function
void str_test()
{
        int i=3500;
        char id[5]={0, };       
        char age[4]="19";

        // integer --> char
        itoa(i, id);
        printf("itoa(%d) --> %s\n",i, id);

        // char --> integer
        i = atoi(age);
        printf("atoi(%s) --> %d\n",age, i);

}

int main()
{
        HEAD sheader, rheader;
        char msg[BUF_SIZE], rmsg[BUF_SIZE];
        // temporary variable
        char id[5]={0, }, age[4]={0, };
        int off;        // offset

        str_test();

        // Sender  input data
        memset(&sheader, 0, sizeof(HEAD));
        sheader.id = 3500;
        strcpy(sheader.name, "CAPTAIN");
        sheader.age = 19;
        strcpy(sheader.email, "CAPTAIN@dimigo.hs.kr");

        printf("[SEND] %d %s %d %s \n", sheader.id, sheader.name, sheader.age, sheader.email);
// make msg
        memset(&msg, 0, BUF_SIZE);
        // id 
        itoa(sheader.id, id);
        memcpy(&msg, &id, sizeof(int));
        off = sizeof(int);
        // name
        memcpy(&msg[off], &sheader.name, sizeof(sheader.name));
        off += sizeof(sheader.name);

        // age
        itoa(sheader.age, age);
        memcpy(&msg[off], &age, sizeof(int));
        off += sizeof(int);

        // email
        memcpy(&msg[off], &sheader.email, sizeof(sheader.email));

        // Send() to peer
        //     --> write()
        //
        memset(&rmsg, 0, BUF_SIZE);     
        memcpy(&rmsg, &msg, BUF_SIZE);  
        
        //
        // Receive from peer
        //      --> read()

        memset(&rheader, 0, sizeof(HEAD));
        // Get id
        memcpy(&id, &rmsg, sizeof(int));
        rheader.id = atoi(id);
        off = sizeof(int);

        // Get name
        memcpy(&rheader.name, &rmsg[off], sizeof(rheader.name));
        off += sizeof(rheader.name);

        // Get age
        memcpy(&age, &rmsg[off], sizeof(int));
        rheader.age = atoi(age);
        off += sizeof(int);
        // Get email
        memcpy(&rheader.email, &rmsg[off], sizeof(rheader.email));

        printf("[RECV] %d %s %d %s \n", rheader.id, rheader.name, rheader.age, rheader.email);

}
