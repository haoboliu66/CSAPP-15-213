


int m(char *s, int x){

s++;

return x + 10;
}

int main(){

int x = 25000;

char *s = "abcdefg";

m(s, x);

}
