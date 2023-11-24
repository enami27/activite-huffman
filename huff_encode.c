
#include "arbrebin.h"
#include "bfile.h"
#include "fap.h"
#include "huffman_code.h"
#include <assert.h>
#include <stdio.h>

typedef struct {
    int tab[256];
} TableOcc_t;

struct code_char HuffmanCode[256];

void ConstruireTableOcc(FILE *fichier, TableOcc_t *TableOcc) {
    int c;
    // init
    for (int i = 0; i < 256; i++) {
        TableOcc->tab[i] = 0;
    }
    // lecture fichier
    c = fgetc(fichier);
    while (c != EOF) {
        TableOcc->tab[c]++;
        c = fgetc(fichier);
    }

    // affichage resultat
    for (int i = 0; i < 256; i++) {
        if (TableOcc->tab[i] != 0)
            printf("Occurences du caractère %c (code %d) : %d\n", i, i,
                   TableOcc->tab[i]);
    }
}


fap InitHuffman(TableOcc_t *TableOcc) {
    fap file = fapVide();
    for (int i = 0; i < 256; i++) {
        if (TableOcc->tab[i] != 0) {
            Arbre noeud = NouveauNoeud(i, TableOcc->tab[i], ArbreVide());
            file = Inserer(file, noeud, TableOcc->tab[i]);
        }
    }
    return file;
}


Arbre ConstruireArbre(fap file) {
    while (!est_fap_vide(file)) {
        Arbre n1, n2;
        int priorite1, priorite2;

        file = extraire(file, &n1, &priorite1);
        if (est_fap_vide(file)) {
            return n1; 
        }
        file = extraire(file, &n2, &priorite2);

        Arbre nouveau = NouveauNoeud(n1, '\0', n2);
        int nouvellePriorite = priorite1 + priorite2;

        file = inserer(file, nouveau, nouvellePriorite);
    }

    return ArbreVide(); 
}



void ConstruireCode(Arbre huff) {
    int pileNœuds[256]; // stocke adresses des noeuds de l'arbre
    int pileCodes[256][256]; // stocke code huffman
    int longueurs[256]; // stocke longueur code
    int pileTaille = 0; 

    if (!EstVide(huff)) {
        pileNœuds[pileTaille] = (int)huff;
        longueurs[pileTaille] = 0;
        pileTaille++;
    }

    while (pileTaille > 0) {
        pileTaille--;
        Arbre noeudCourant = (Arbre)pileNœuds[pileTaille];
        int longueurCourante = longueurs[pileTaille];
        int *codeCourant = pileCodes[pileTaille];

        if (EstVide(FilsGauche(noeudCourant)) && EstVide(FilsDroit(noeudCourant))) {
            int caractere = Etiq(noeudCourant);
            HuffmanCode[caractere].lg = longueurCourante;
            for (int i = 0; i < longueurCourante; i++) {
                HuffmanCode[caractere].code[i] = codeCourant[i];
            }
        } else {
            if (!EstVide(FilsDroit(noeudCourant))) {
                pileNœuds[pileTaille] = (int)FilsDroit(noeudCourant);
                memcpy(pileCodes[pileTaille], codeCourant, longueurCourante * sizeof(int));
                pileCodes[pileTaille][longueurCourante] = 1;
                longueurs[pileTaille] = longueurCourante + 1;
                pileTaille++;
            }
            if (!EstVide(FilsGauche(noeudCourant))) {
                pileNœuds[pileTaille] = (int)FilsGauche(noeudCourant);
                memcpy(pileCodes[pileTaille], codeCourant, longueurCourante * sizeof(int));
                pileCodes[pileTaille][longueurCourante] = 0;
                longueurs[pileTaille] = longueurCourante + 1;
                pileTaille++;
            }
        }
    }
}

void Encoder(FILE *fic_in, FILE *fic_out, Arbre ArbreHuffman) {
    int c;
    BFILE *bf;

    EcrireArbre(fic_out, ArbreHuffman);

    bf = bstart(fic_out, "w");

    c = fgetc(fic_in);
    while (c != EOF) {
        struct code_char code = HuffmanCode[c];
        for (int i = 0; i < code.lg; i++) {
            bitwrite(bf, code.code[i]);
        }
        c = fgetc(fic_in);
    }

    bstop(bf);
}

int main(int argc, char *argv[]) {

    TableOcc_t TableOcc;
    FILE *fichier;
    FILE *fichier_encode;

    fichier = fopen(argv[1], "r");
    /* Construire la table d'occurences */
    ConstruireTableOcc(fichier, &TableOcc);
    fclose(fichier);

    /* Initialiser la FAP */
    fap file = InitHuffman(&TableOcc);

    /* Construire l'arbre d'Huffman */
    Arbre ArbreHuffman = ConstruireArbre(file);

        AfficherArbre(ArbreHuffman);

    /* Construire la table de codage */
    ConstruireCode(ArbreHuffman);

    /* Encodage */
    fichier = fopen(argv[1], "r");
    fichier_encode = fopen(argv[2], "w");

    Encoder(fichier, fichier_encode, ArbreHuffman);

    fclose(fichier_encode);
    fclose(fichier);

    return 0;
}
