#include <stdio.h>

#include "arbrebin.h"
#include "assert.h"
#include "bfile.h"

void Decoder(FILE *fichier_encode, Arbre ArbreHuffman) {
    BFILE *bf;
    int bit;
    Arbre courant = ArbreHuffman;

    bf = bstart(fichier_encode, "r");

    while ((bit = bitread(bf)) != -1) {
        if (bit == 0) {
            courant = FilsGauche(courant);
        } else {
            courant = FilsDroit(courant);
        }

        if (EstVide(FilsGauche(courant)) && EstVide(FilsDroit(courant))) {
            printf("%c", Etiq(courant));
            courant = ArbreHuffman; 
        }
    }

    bstop(bf);
}


int main(int argc, char *argv[]) {
    FILE *fichier_encode = fopen(argv[1], "r");

    Arbre ArbreHuffman = LireArbre(fichier_encode);

    Decoder(fichier_encode, ArbreHuffman);

    fclose(fichier_encode);

    return 0;
}
