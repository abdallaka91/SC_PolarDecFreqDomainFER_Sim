/* ==========================================================================
 Fichier tableaux1.c
 Manipulation des tableaux 1D.-

 ===========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#define MAX1 50                 // Constante Taille maximum tableau 1D 
int main()
{
  int t[MAX1];                  // Déclaration t tableau 1D d'entiers 
    int t2[MAX1];                  // Déclaration t tableau 1D d'entiers 
  int n;                        // Dimension (taille) tableau 1D 
  int i, j,tmp, u,v,  valMin,posMin,valMax,posMax;    
  int  pos ,p, pla;                   
 // Affichage titre
 printf("\n ***************** Manipulation Tableaux 1D ************* \n\n");
 // ------------- Tableaux 1D : Saisie -------------------
 printf("\n\n --> Saisie du tableau 1D \n");
 // Saisie de la taille du tableau 
 printf("\n Saisir la taille du tableau : ");
 scanf("%d",&n);
 printf("\n"); 
 // Saisie des valeurs du tableau 1D
 for (i=0;i<n;i++){
   printf("Entrer t[%d]:",i);
   scanf("%d",&t[i]);
 }
 // -------------- Tableaux 1D : Affichage Vertical -------
 printf("\n\n --> Affichage vertical du tableau 1D de base \n\n ");
 for (i=0; i<n;i++){
    printf("t[%d] = %d \n ",i,t[i]);
 }
     printf(" \n\n\n ");
	 /*
 // ----------- Traitement 1D :  Partie à completer -------
 // ----------- affichage à l'envers -------
 printf("affichage a l'envers   \n ");
 for (i=n-1; i>=0;i--){
    printf(" %d  \n ",t[i]);
 }
  // ----------- affichage valeurs pairs -------
 printf("affichage valeurs pairs   \n ");
 for (i=0; i<n;i++){
 	if(t[i]%2==0){
    printf(" %2d  \n ",t[i]);
     }
 }
   // ----------- Permutation des valeurs du tableau -------
   printf("Entrer les indices  ");
   scanf("%d %d",&u, &v);
   if (u>=n || v>=n)
   {
 	 printf("saisir des indices inferieurs a n   \n ");}
 	 else{
      tmp=t[u];
      t[u]=t[v];
      t[v]=tmp;
      for (i=0; i<n;i++){
      printf("t[%d] = %d \n ",i,t[i]);
    }
   }
    // ----------- Affichage les valeurs min et max -------
    valMin=t[0];
	posMin= 0;
	valMax=t[0];
	posMax= 0;
 for (i=1; i<n;i++){
 	if (t[i]<valMin){
 	 valMin=t[i];
	  posMin= i;}
	if (t[i]>valMax){
 	  valMax=t[i];
	  posMax= i;}
  }
    printf(" La valeur minimale est %2d avec indice %2d \n ",valMin,posMin);
    printf(" La valeur maximale est %2d avec indice %2d \n ",valMax,posMax);  
    // ----------- Extraction des valeurs -------
    printf("Entrer la position debut  ");
    scanf("%d",&pos);
    printf("Entrer la taille du t2  ");
   scanf("%d ",&p);
   j=0;
 for (i=pos; i<=pos+p-1;i++){
 	t2[j]=t[i];
 	printf(" %d \n ",t2[j]);
 	j=j+1;
 }*/
    // ----------- Décalage à gauche -------
    printf("Decalage d'une position a gauche \n  ");
   for (i=0; i<n-1;i++){
    printf("t[%d] = %d \n ",i,t[i+1]);
   }

 printf("\n\n");
 system("pause");
 return 0;
}// fin main
