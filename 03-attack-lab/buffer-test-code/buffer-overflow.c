
#include <stdio.h>



void echo(){

char buf[8];
gets(buf);
puts(buf);

}


int main(){

printf("test buffer overflow......\n");

echo();


}
