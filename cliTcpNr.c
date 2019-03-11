/* cliTCPIt.c - Exemplu de client TCP
   Trimite un numar la server; primeste de la server numarul incrementat.
         
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>

/* codul de eroare returnat de anumite apeluri */
extern int errno;
/* portul de conectare la server*/
int port;
/* mysql */
int functionVal(char valoare_comanda[]);
int main(int argc, char *argv[])
{
  int sd;                    // descriptorul de socket
  struct sockaddr_in server; // structura folosita pentru conectare
                             // mesajul trimis
  int nr = 0;
  char username_client[100];
  char password_client[100];
  char valoare_login[100];
  char comanda[100];

  char register_username[100];
  char register_password[100];

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
  {
    printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
    return -1;
  }

  /* stabilim portul */
  port = atoi(argv[2]);

  /* cream socketul */
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("Eroare la socket().\n");
    return errno;
  }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons(port);

  /* ne conectam la server */
  if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[client]Eroare la connect().\n");
    return errno;
  }
  int login_value = 1;
  int eroare_comanda = 1;
  /* citirea mesajului */
  printf("[client]Comenzi disponibile: \n");
  printf("> 'register' ( inregistrare cont )\n");
  printf("> 'login'    ( logare cont )\n");
  printf("> 'exit'     ( iesire din program )\n");
  printf("[client]Introduceti comanda: ");
  fflush(stdout);
  int compro = 1;
  while (compro == 1)
  {
    bzero(&comanda, sizeof(comanda));
    read(0, comanda, sizeof(comanda));
    comanda[strlen(comanda) - 1] = '\0';
    if (strcmp(comanda, "login") == 0 or strcmp(comanda, "register") == 0 or strcmp(comanda, "exit") == 0)
      compro = 0;
    else
    {
      printf("Nu ati introdus comanda potrivita\n");
      printf("Induceti din nou comanda: ");
      fflush(stdout);
    }
  }

  if (write(sd, &comanda, sizeof(comanda)) <= 0)
  {
    perror("[client]Eroare la write() spre server.\n");
    return errno;
  }

  if (strcmp(comanda, "register") == 0)
  {

    bzero(&register_username, sizeof(register_username));
    bzero(&register_password, sizeof(register_password));

    printf("[register]Introduceti un username: ");

    fflush(stdout);
    read(0, register_username, sizeof(register_username));
    register_username[strlen(register_username) - 1] = '\0';

    /* trimiterea mesajului la server */
    if (write(sd, &register_username, sizeof(register_username)) <= 0)
    {
      perror("[client]Eroare la write() spre server.\n");
      return errno;
    }
    printf("[register]Introduceti parola: ");
    fflush(stdout);
    read(0, register_password, sizeof(register_password));
    if (write(sd, &register_password, sizeof(register_password)) <= 0)
    {
      perror("[client]Eroare la write() spre server.\n");
      return errno;
    }
  }

  if (strcmp(comanda, "login") == 0 or strcmp(comanda, "register") == 0)
  {
    while (login_value == 1)
    {
      bzero(&username_client, sizeof(username_client));
      bzero(&password_client, sizeof(password_client));

      printf("[login]Introduceti un username: ");

      fflush(stdout);
      read(0, username_client, sizeof(username_client));
      username_client[strlen(username_client) - 1] = '\0';

      /* trimiterea mesajului la server */
      if (write(sd, &username_client, sizeof(username_client)) <= 0)
      {
        perror("[client]Eroare la write() spre server.\n");
        return errno;
      }
      printf("[login]Introduceti parola: ");
      fflush(stdout);
      read(0, password_client, sizeof(password_client));
      //password_client[strlen(password_client) - 1] = '\0';
      if (write(sd, &password_client, sizeof(password_client)) <= 0)
      {
        perror("[client]Eroare la write() spre server.\n");
        return errno;
      }

      /* citirea raspunsului dat de server 
     (apel blocant pina cind serverul raspunde) */
      if (read(sd, &valoare_login, sizeof(valoare_login)) < 0)
      {
        perror("[client]Eroare la read() de la server.\n");
        return errno;
      }
      /* afisam mesajul primit */
      printf("[SERVER] : %s \n", valoare_login);

      if (strcmp(valoare_login, "Te-ai conectat cu succes!") == 0)
        login_value = 0;
    }
  }

  if (login_value == 0)
  {

    int logare_succes = 1;
    //char valoare_comanda[100];
    char valoare_comanda[250];
    printf("\n\n");
    printf("-----------------------------------------------------------\n");
    printf("[%s]Comenzi disponibile: \n", username_client);
    printf("**COMENZI USERS :**\n");
    printf("> 'add music ( adauga o melodie in top )\n");
    printf("> 'top music'     ( afisarea topului dupa voturi )\n");
    printf("> 'top categorie' ( afisarea topului dupa o categorie )\n");
    printf("> 'voteaza'       ( voteaza o melodie )\n");
    printf("> 'comentarii'    ( adauga un comentariu la o melodie )\n");
    printf("> 'exit'          ( iesi din aplicatie )\n");
    printf("**COMENZI ADMINISTRATOR SUPLIMENTARE :**\n");
    printf("> 'delete'        ( sterge o melodie)\n");
    printf("> 'drept votare'  ( restrictioneaza dreptul de votare)\n");
    printf("-----------------------------------------------------------\n\n");

    while (logare_succes == 1)
    {
      printf("[%s]Introduceti comanda: ", username_client);
      fflush(stdout);

      bzero(&valoare_comanda, sizeof(valoare_comanda));
      read(0, valoare_comanda, sizeof(valoare_comanda));
      valoare_comanda[strlen(valoare_comanda) - 1] = '\0';
      //printf("Valoare comanda: %s\n", valoare_comanda);
      if (write(sd, &valoare_comanda, sizeof(valoare_comanda)) <= 0)
      {
        perror("[client]Eroare la write() spre server.\n");
        return errno;
      }

      if (functionVal(valoare_comanda) != 0)
      {
        printf("Nu ati introdus ce trebuie .\n");
      }
      if (strcmp(valoare_comanda, "exit") == 0)
        logare_succes = 0;
      printf("\n");

      //add music
      if (strcmp(valoare_comanda, "add music") == 0)
      {
        printf("** SISTEM ADAUGARE MELODIE **\n");
        printf("Numele melodiei: ");
        fflush(stdout);
        char nume_melodie[200];
        bzero(&nume_melodie, sizeof(nume_melodie));
        read(0, nume_melodie, sizeof(nume_melodie));
        nume_melodie[strlen(nume_melodie) - 1] = '\0';

        if (write(sd, &nume_melodie, sizeof(nume_melodie)) <= 0)
        {
          perror("[client]Eroare la write() spre server.\n");
          return errno;
        }

        printf("Link-ul melodiei: ");
        fflush(stdout);
        char link_melodie[200];
        scanf("%s", link_melodie);

        if (write(sd, &link_melodie, sizeof(link_melodie)) <= 0)
        {
          perror("[client]Eroare la write() spre server.\n");
          return errno;
        }

        printf("Melodia este genul rock ? (y/n) \n ");
        printf("Raspuns: ");
        fflush(stdout);
        char rock_melodie[200];
        int rock = 1;
        while (rock == 1)
        {
          scanf("%s", rock_melodie);
          if (strcmp(rock_melodie, "y") == 0 or strcmp(rock_melodie, "n") == 0)
          {
            rock = 0;
          }
          else
          {
            printf("Puteti introduce doar 'y' sau 'n' !\n");
            printf("Adaugati un alt raspuns: ");
          }
        }
        // printf("AFARA DIN WHILE");
        if (write(sd, &rock_melodie, sizeof(rock_melodie)) <= 0)
        {
          perror("[client]Eroare la write() spre server.\n");
          return errno;
        }

        printf("Melodia este genul dance ? (y/n) \n ");
        printf("Raspuns: ");
        fflush(stdout);
        char dance_melodie[200];
        int dance = 1;
        while (dance == 1)
        {
          fflush(stdin);

          scanf("%s", dance_melodie);

          if (strcmp(dance_melodie, "y") == 0 or strcmp(dance_melodie, "n") == 0)
          {
            dance = 0;
          }
          else
          {
            printf("Puteti introduce doar 'y' sau 'n' !\n");
            printf("Adaugati un alt raspuns: ");
          }
        }
        if (write(sd, dance_melodie, sizeof(dance_melodie)) <= 0)
        {
          perror("[client]Eroare la write() spre server.\n");
          return errno;
        }

        printf("Melodia este genul hiphop ? (y/n) \n ");
        printf("Raspuns: ");
        fflush(stdout);
        char hiphop_melodie[200];
        int hiphop = 1;
        while (hiphop == 1)
        {

          scanf("%s", hiphop_melodie);
          if (strcmp(hiphop_melodie, "y") == 0 or strcmp(hiphop_melodie, "n") == 0)
            hiphop = 0;
          else
          {
            printf("Puteti introduce doar 'y' sau 'n' !\n");
            printf("Adaugati un alt raspuns: ");
          }
        }

        if (write(sd, &hiphop_melodie, sizeof(hiphop_melodie)) <= 0)
        {
          perror("[client]Eroare la write() spre server.\n");
          return errno;
        }
      }
      //add music finish

      if (strcmp(valoare_comanda, "voteaza") == 0)
      {
        char nume_mel[200];
        bzero(&nume_mel, sizeof(nume_mel));
        printf("Introduceti melodia pe care vreti sa o votati:\n");
        printf("Raspuns: ");
        fflush(stdout);
        read(0, nume_mel, sizeof(nume_mel));
        nume_mel[strlen(nume_mel) - 1] = '\0';
        if (write(sd, &nume_mel, sizeof(nume_mel)) <= 0)
        {
          perror("[client]Eroare la write() spre server in voteaza.\n");
          return errno;
        }
      }
      //delete
      if (strcmp(valoare_comanda, "delete") == 0)
      {
        int case_delete = 1;

        char delete_melodie[200];
        bzero(&delete_melodie, sizeof(delete_melodie));

        printf("[Introduceti numele melodiei pe care vreti sa o stergeti: ");

        fflush(stdout);
        read(0, delete_melodie, sizeof(delete_melodie));
        delete_melodie[strlen(delete_melodie) - 1] = '\0';

        write(sd, delete_melodie, sizeof(delete_melodie));
        char grad_delete[200];

        read(sd, &grad_delete, sizeof(grad_delete));
        if (strcmp(grad_delete, "ok") == 0)
          printf("Ati sters cu succes melodia!\n");
        else
          printf("Nu aveti grad de administrator pentru a sterge melodia!\n");
      }
      //delete finish

      //top music
      if (strcmp(valoare_comanda, "top music") == 0)
      {
        int i = 1;
        char afiseaza_top[200];
        char *p;
        read(sd, &afiseaza_top, sizeof(afiseaza_top));
        afiseaza_top[strlen(afiseaza_top) - 1] = '\0';
        p = strtok(afiseaza_top, "@");
        printf("\n **TOPUL TUTUROR MELODIILOR ESTE**\n");

        while (p)
        {
          printf("%d", i);
          printf(". ");
          printf("%s", p);
          printf("\n");
          i++;
          p = strtok(NULL, "@");
        }
      }
      if (strcmp(valoare_comanda, "drept votare") == 0)
      {
        char utilizator_votare[200];
        printf("Indoduceti numele utilizatorului caruia doriti sa ii schimbati permisiunea de a vota!\n");
        printf("Raspuns: ");
        scanf("%s", utilizator_votare);
        if (write(sd, utilizator_votare, sizeof(utilizator_votare)) <= 0)
        {
          perror("[client]Eroare la write() spre server.\n");
          return errno;
        }
        printf("Introduceti 'x' pentru a opri permisiunea de a vota sau 'o' pentru a avea permisiunea de a vota\n");
        printf("Initial orice user are dreptul de a vota\n");
        printf("Raspuns: ");
        char permisiune_votare[200];
        int perm_vot = 1;
        while (perm_vot == 1)
        {

          scanf("%s", permisiune_votare);
          if (strcmp(permisiune_votare, "x") == 0 or strcmp(permisiune_votare, "o") == 0)
            perm_vot = 0;
          else
          {
            printf("Puteti introduce doar 'x' sau 'o' !\n");
            printf("Adaugati un alt raspuns: ");
          }
          if (write(sd, permisiune_votare, sizeof(permisiune_votare)) <= 0)
          {
            perror("[client]Eroare la write() spre server.\n");
            return errno;
          }
        }
        char raspuns_grad[200];
        read(sd, &raspuns_grad, sizeof(raspuns_grad));
        printf("%s", raspuns_grad);
      }
      if (strcmp(valoare_comanda, "top categorie") == 0)
      {
        char nume_categorie[200];
        printf("Introduceti numele categoriei pentru care vreti sa afisati topul !\n");
        printf("Optiunile sunt: 'rock' , 'dance' , 'hiphop'  !\n");
        printf("Raspuns: ");
        fflush(stdout);
        int categorie = 1;
        while (categorie == 1)
        {
          scanf("%s", nume_categorie);
          if (strcmp(nume_categorie, "rock") == 0 or strcmp(nume_categorie, "dance") == 0 or strcmp(nume_categorie, "hiphop") == 0)
            categorie = 0;
          else
          {
            printf("Nu ati introdus ce trebuie, optiunile sunt mai sus !\n");
            printf("Raspuns: ");
          }
        }

        write(sd, &nume_categorie, sizeof(nume_categorie));
        int i = 1;
        char afiseaza_top[200];
        char *p;

        read(sd, &afiseaza_top, sizeof(afiseaza_top));
        afiseaza_top[strlen(afiseaza_top) - 1] = '\0';

        p = strtok(afiseaza_top, "@");
        printf("\n **Topul categoriei %s este**\n", nume_categorie);

        while (p)
        {
          printf("%d", i);
          printf(". ");
          printf("%s", p);
          printf("\n");
          i++;
          p = strtok(NULL, "@");
        }
      }

      // top music finish
      if (strcmp(valoare_comanda, "comentarii") == 0)
      {
        char comanda_comentariu[200];
        int comentariu = 1;

        printf("Introduceti 'adauga comentariu' pentru a adauga un comentariu!\n");
        printf("Introduceti 'afiseaza comentariu' pentru a afisa comentariile unei melodii\n");
        printf("Comenzile se efectueaza pentru o anumita melodie citita de la tastatura\n");
        printf("Raspuns: ");
        fflush(stdout);
        while (comentariu == 1)
        {
          bzero(&comanda_comentariu, sizeof(comanda_comentariu));
          read(0, comanda_comentariu, sizeof(comanda_comentariu));
          comanda_comentariu[strlen(comanda_comentariu) - 1] = '\0';
          if (strcmp(comanda_comentariu, "adauga comentariu") == 0 or strcmp(comanda_comentariu, "afiseaza comentariu") == 0)
            comentariu = 0;
          else
          {
            printf("Comanda incorecta!\n");
            printf("Raspuns: ");
            fflush(stdout);
          }
        }
        write(sd, comanda_comentariu, sizeof(comanda_comentariu));

        if (strcmp(comanda_comentariu, "adauga comentariu") == 0)
        {
          char nume[200];
          char com[200];
          printf("Introduceti numele melodiei: ");
          fflush(stdout);
          bzero(&nume, sizeof(nume));
          read(0, nume, sizeof(nume));
          nume[strlen(nume) - 1] = '\0';
          bzero(&com, sizeof(com));
          printf("Introduceti comentariul: ");
          fflush(stdout);
          read(0, com, sizeof(com));
          com[strlen(com) - 1] = '\0';

          write(sd, nume, sizeof(nume));
          write(sd, com, sizeof(com));
        }
        if (strcmp(comanda_comentariu, "afiseaza comentariu") == 0)
        {
          char final_afisare[200];
          char nume_afisare[200];
          printf("Introduceti numele melodiei: ");
          fflush(stdout);
          bzero(&nume_afisare, sizeof(nume_afisare));
          read(0, nume_afisare, sizeof(nume_afisare));
          nume_afisare[strlen(nume_afisare) - 1] = '\0';
          write(sd, nume_afisare, sizeof(nume_afisare));
          bzero(&final_afisare, sizeof(final_afisare));
          read(sd, &final_afisare, sizeof(final_afisare));
          printf("\n");
          printf("Afisare : %s\n",final_afisare);
          fflush(stdout);
        }
      }
    }
  }

  close(sd);
}
int functionVal(char valoare_comanda[])
{
  if (strcmp(valoare_comanda, "add music") == 0 or strcmp(valoare_comanda,"comentarii") == 0 or strcmp(valoare_comanda, "top music") == 0 or strcmp(valoare_comanda, "top categorie") == 0 or strcmp(valoare_comanda, "voteaza") == 0 or strcmp(valoare_comanda, "delete") == 0 or strcmp(valoare_comanda, "exit") == 0 or strcmp(valoare_comanda, "drept votare") == 0)
  {
    return 0;
  }
}