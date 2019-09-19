/* scanf example */
#include <stdio.h>

int main() {
    //   char str [80];
    //   int i;

    //   printf ("Enter your family name: ");
    //   scanf ("%s",str);
    //   printf ("Enter your age: ");
    //   scanf ("%d",&i);
    //   printf ("Mr. %s , %d years old.\n",str,i);
    //   printf ("Enter a hexadecimal number: ");
    //   scanf ("%x",&i);
    //   printf ("You have entered %#x (%d).\n",i,i);

    int i1, i2, i3, i4, i5, i6;
    char format[] = "%d %d %d %d %d %d";
    int res = sscanf("-1 2 3 4 5 6", format, &i1, &i2, &i3, &i4, &i5, &i6);
    printf("%d\n", res);
    printf("%d %d %d %d %d %d\n", i1, i2, i3, i4, i5, i6);
    return 0;
}