
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <cstring>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <mysql/mysql.h>

void finish_with_error(MYSQL *con)
{
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  exit(1);
}
void login_mysql(char usr[], char pass[], int &nr_rows);
void register_mysql(char usr[], char pass[]);
void returnare_mysql(char usr[], char pass[], char &nr_rows);
void addMusic_mysql(char nume_melodie[], char link_melodie[], char rock_melodie[], char dance_melodie[], char hiphop_melodie[]);
void top_mysql(char top_melodie[]);
void topCategorie_mysql(char top_melodie[], char nume_categorie[]);
void voteaza_mysql(char nume_melodie[], int &nr_voturi);
void delete_mysql(char delete_melodie[]);
void admin_delete_mysql(char grad_delete[], char user[]);
void permisiune_0_mysql(char user[]);
void permisiune_1_mysql(char user[]);
int scot_votare_mysql(char perm[]);
void verificare_permisiune_mysql(char perm[], char user[]);
void scot_votare_mysql(char perm[], int vot);
void adaug_votare_mysql(char nume_mel[], int vot);
void adaug_comentariu_mysql(char nume[], char com[]);
void selectez_comentariu_mysql(char nume[],char final[]);
/* portul folosit */
#define PORT 2909

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData
{
  int idThread; //id-ul thread-ului tinut in evidenta de acest program
  int cl;       //descriptorul intors de accept
} thData;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
void raspunde(void *);

int main(int argc, char **argv)
{

  MYSQL *con = mysql_init(NULL);

  if (con == NULL)
  {
    fprintf(stderr, "%s\n", mysql_error(con));
    exit(1);
  }

  if (mysql_real_connect(con, "den1.mysql5.gear.host", "topmusic", "Yx3Kv9~i?9C7", "topmusic", 0, NULL, 0))
  {
    printf("Conectare cu succes la MySql!\n");
  }
  else
  {
    printf("Nu te-ai putut conecta cu succes la MySql!\n");
  }

  struct sockaddr_in server; // structura folosita de server
  struct sockaddr_in from;
  int nr; //mesajul primit de trimis la client
  int sd; //descriptorul de socket
  int pid;
  pthread_t th[100]; //Identificatorii thread-urilor care se vor crea
  int i = 0;

  /* crearea unui socket */
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("[server]Eroare la socket().\n");
    return errno;
  }
  /* utilizarea optiunii SO_REUSEADDR */
  int on = 1;
  setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

  /* pregatirea structurilor de date */
  bzero(&server, sizeof(server));
  bzero(&from, sizeof(from));

  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
  server.sin_family = AF_INET;
  /* acceptam orice adresa */
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  /* utilizam un port utilizator */
  server.sin_port = htons(PORT);

  /* atasam socketul */
  if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[server]Eroare la bind().\n");
    return errno;
  }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen(sd, 2) == -1)
  {
    perror("[server]Eroare la listen().\n");
    return errno;
  }
  printf("[SERVER]Asteptam conexiunea clientuluila portul: %d...\n", PORT);
  fflush(stdout);

  /* servim in mod concurent clientii...folosind thread-uri */
  while (1)
  {
    int client;
    thData *td; //parametru functia executata de thread
    int length = sizeof(from);

    //client= malloc(sizeof(int));
    /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
    if ((client = accept(sd, (struct sockaddr *)&from, (socklen_t *)&length)) < 0)
    {
      perror("[server]Eroare la accept().\n");
      continue;
    }

    /* s-a realizat conexiunea, se astepta mesajul */

    int idThread; //id-ul threadului
    int cl;       //descriptorul intors de accept

    td = (struct thData *)malloc(sizeof(struct thData));
    td->idThread = i++;
    td->cl = client;

    pthread_create(&th[i], NULL, &treat, td);

  } //while

  mysql_close(con);
}

static void *treat(void *arg)
{
  struct thData tdL;
  tdL = *((struct thData *)arg);
  printf("[CLIENTUL %d] - Asteptam mesajul !\n", tdL.idThread);
  fflush(stdout);
  pthread_detach(pthread_self());

  raspunde((struct thData *)arg);

  /* am terminat cu acest client, inchidem conexiunea */
  close((intptr_t)arg);
  return (NULL);
};

void raspunde(void *arg)
{
  int nr_rows;
  char username[100];
  char password[100];
  char register_username[100];
  char register_password[100];
  char comanda[110];
  struct thData tdL;
  tdL = *((struct thData *)arg);
  int conectare = 1;

  if (read(tdL.cl, &comanda, sizeof(comanda)) <= 0)

  {
    printf("[Thread %d]\n", tdL.idThread);
    perror("Eroare la read() de la client.\n");
  }

  if (strcmp(comanda, "register") == 0)
  { //
    bzero(&register_username, sizeof(register_username));
    bzero(&register_password, sizeof(register_password));

    if (read(tdL.cl, &register_username, sizeof(register_username)) <= 0)
    {
      printf("[Thread %d]\n", tdL.idThread);
      perror("Eroare la read() de la client.\n");
    }

    if (read(tdL.cl, &register_password, sizeof(register_password)) <= 0)
    {
      printf("[Thread %d]\n", tdL.idThread);
      perror("Eroare la read() de la client.\n");
    }
    register_mysql(register_username, register_password);
  }

  if (strcmp(comanda, "login") == 0 or strcmp(comanda, "register") == 0)
  {
    while (conectare == 1)
    {
      bzero(&username, sizeof(username));
      bzero(&password, sizeof(password));

      if (read(tdL.cl, &username, sizeof(username)) <= 0)
      {
        printf("[Thread %d]\n", tdL.idThread);
        perror("Eroare la read() de la client.\n");
        conectare = 0;
      }

      if (read(tdL.cl, &password, sizeof(password)) <= 0)
      {
        printf("[Thread %d]\n", tdL.idThread);
        perror("Eroare la read() de la client.\n");
        conectare = 0;
      }

      /*pregatim mesajul de raspuns */
      printf("[Thread %d]Usernameul este: %s\n", tdL.idThread, username);
      printf("[Thread %d]Parola este: %s", tdL.idThread, password);
      /* returnam mesajul clientului */

      char raspuns[100];
      login_mysql(username, password, nr_rows);

      if (nr_rows > 0)
        strcpy(raspuns, "Te-ai conectat cu succes!");
      else
        strcpy(raspuns, "Username sau password incorecte!");

      if (write(tdL.cl, &raspuns, sizeof(raspuns)) <= 0)
      {
        printf("[Thread %d] ", tdL.idThread);
        perror("[Thread]Eroare la write() catre client.\n");
        conectare = 0;
      }
      else
      {
        printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", tdL.idThread);
        printf("\n");
        if (nr_rows > 0)
          conectare = 0;
      }
    }
    //while conectare
  }

  //-------------------------------------------------------------
  if (conectare == 0)
  {
    char valoare_comanda[250];

    int logare_succes = 1;

    while (logare_succes == 1)
    {
      bzero(&valoare_comanda, sizeof(valoare_comanda));
      if (read(tdL.cl, &valoare_comanda, sizeof(valoare_comanda)) <= 0)
      {
        printf("[Thread %d]\n", tdL.idThread);
        perror("Eroare la read() de la client.\n");
      }

      printf("[CLIENT %d] Ati introdus : %s\n", tdL.idThread, valoare_comanda);

      int valoareSwitch;
      valoareSwitch = 0;
      char valoare_negativa[100] = "OK";

      if (strcmp(valoare_comanda, "add music") == 0)
        valoareSwitch = 1;
      else if (strcmp(valoare_comanda, "top music") == 0)
        valoareSwitch = 2;
      else if (strcmp(valoare_comanda, "top categorie") == 0)
        valoareSwitch = 3;
      else if (strcmp(valoare_comanda, "voteaza") == 0)
        valoareSwitch = 4;
      else if (strcmp(valoare_comanda, "delete") == 0)
        valoareSwitch = 5;
      else if (strcmp(valoare_comanda, "drept votare") == 0)
        valoareSwitch = 6;
      else if (strcmp(valoare_comanda, "exit") == 0)
        valoareSwitch = 7;
      else if (strcmp(valoare_comanda, "comentarii") == 0)
        valoareSwitch = 8;
      else
      {
        valoareSwitch = 0;
      }
      printf("v switch: %d\n", valoareSwitch);
      fflush(stdout);
      switch (valoareSwitch)
      {
        /* add music */
      case 1:
      {
        char nume_melodie[200];
        char link_melodie[200];
        char rock_melodie[200];
        char dance_melodie[200];
        char hiphop_melodie[200];
        read(tdL.cl, &nume_melodie, sizeof(nume_melodie));
        read(tdL.cl, &link_melodie, sizeof(link_melodie));
        read(tdL.cl, &rock_melodie, sizeof(rock_melodie));
        read(tdL.cl, &dance_melodie, sizeof(dance_melodie));
        read(tdL.cl, &hiphop_melodie, sizeof(hiphop_melodie));
        printf("%s", nume_melodie);
        addMusic_mysql(nume_melodie, link_melodie, rock_melodie, dance_melodie, hiphop_melodie);

        break;
      }

      /* top music */
      case 2:
      {
        char afiseaza_top[200];
        strcpy(afiseaza_top, "");
        top_mysql(afiseaza_top);

        if (write(tdL.cl, &afiseaza_top, sizeof(afiseaza_top)) <= 0)
        {
          printf("[Thread switch%d] ", tdL.idThread);
          perror("[Thread]Eroare la write() catre client.\n");
          close(tdL.cl);
          return;
        }

        break;
      }
      /* top categorie */
      case 3:
      {
        char afiseaza_top[200];
        char nume_categorie[200];
        read(tdL.cl, &nume_categorie, sizeof(nume_categorie));

        strcpy(afiseaza_top, "");
        topCategorie_mysql(afiseaza_top, nume_categorie);

        if (write(tdL.cl, &afiseaza_top, sizeof(afiseaza_top)) <= 0)
        {
          printf("[Thread switch%d] ", tdL.idThread);
          perror("[Thread]Eroare la write() catre client.\n");
          close(tdL.cl);
          return;
        }

        break;
      }

      /* voteaza */
      case 4:
      {
        char nume_mel[200];
        char verificare_permisiune[200];
        int vot;
        bzero(&nume_mel, sizeof(nume_mel));
        bzero(&verificare_permisiune, sizeof(verificare_permisiune));
        if (read(tdL.cl, &nume_mel, sizeof(nume_mel)) <= 0)
        {
          printf("[Thread %d]\n", tdL.idThread);
          perror("Eroare la read() de la client in voteaza switch.\n");
          close(tdL.cl);
          return;
        }

        verificare_permisiune_mysql(verificare_permisiune, username);
        if (verificare_permisiune[0] == '1')
        {
          vot = scot_votare_mysql(nume_mel);
          vot++;
          adaug_votare_mysql(nume_mel, vot);
        }
        break;
      }

      /* delete */
      case 5:
      {
        int nr_rows1;
        char grad_delete[200];
        char delete_melodie[200];
        char raspuns_delete[200];
        int case_delete = 1;

        bzero(&delete_melodie, sizeof(delete_melodie));
        bzero(&grad_delete, sizeof(grad_delete));
        bzero(&raspuns_delete, sizeof(raspuns_delete));
        read(tdL.cl, &delete_melodie, sizeof(delete_melodie));
        admin_delete_mysql(grad_delete, username);

        if (strcmp(grad_delete, "admin") == 0)
        {
          delete_mysql(delete_melodie);
          strcpy(raspuns_delete, "ok");
        }
        else
          strcpy(raspuns_delete, "prost");

        write(tdL.cl, &raspuns_delete, sizeof(raspuns_delete));

        break;
      }

      /* drept_votare */
      case 6:
      {
        char utilizator_votare[200];
        char permisiune_votare[200];
        char grad_delete[200];
        bzero(&utilizator_votare, sizeof(utilizator_votare));
        bzero(&permisiune_votare, sizeof(permisiune_votare));
        bzero(&grad_delete, sizeof(grad_delete));

        read(tdL.cl, &utilizator_votare, sizeof(utilizator_votare));
        read(tdL.cl, &permisiune_votare, sizeof(permisiune_votare));

        admin_delete_mysql(grad_delete, username);
        if (strcmp(grad_delete, "admin") == 0)
        {
          if (strcmp(permisiune_votare, "x") == 0)
            permisiune_0_mysql(utilizator_votare);
          else
            permisiune_1_mysql(utilizator_votare);
        }
        char raspuns_grad[200];
        if (strcmp(grad_delete, "admin") == 0)
          strcpy(raspuns_grad, "Ati schimbat permisiunea cu succes\n");
        else
          strcpy(raspuns_grad, "Nu aveti gradul de administrator pentru a schimba permisiunea\n");

        write(tdL.cl, &raspuns_grad, sizeof(raspuns_grad));

        break;
      }

      case 7:
      {

        if (write(tdL.cl, &valoare_comanda, sizeof(valoare_comanda)) <= 0)
        {
          printf("[Thread switch%d] ", tdL.idThread);
          perror("[Thread]Eroare la write() catre client.\n");
        }
        if (strcmp(valoare_comanda, "exit") == 0)
        {
          logare_succes = 0;
        }

        break;
      }
      case 8:
      {
        char comanda_comentariu[200];
        bzero(&comanda_comentariu, sizeof(comanda_comentariu));
        read(tdL.cl, &comanda_comentariu, sizeof(comanda_comentariu));

        if (strcmp(comanda_comentariu, "adauga comentariu") == 0)
        {

          char nume[200];
          char com[200];
          char final[200];
          char intermediar[200];
          bzero(&nume, sizeof(nume));
          bzero(&com, sizeof(com));
          bzero(&final, sizeof(final));
          bzero(&intermediar, sizeof(intermediar));

          read(tdL.cl, &nume, sizeof(nume));
         
          read(tdL.cl, &com, sizeof(com));
         
          selectez_comentariu_mysql( nume,intermediar);
          
          strcpy(final,intermediar);
         
          strcat(final,"\n");
          strcat(final, com);
         
          adaug_comentariu_mysql( nume, final);

       }
        if (strcmp(comanda_comentariu, "afiseaza comentariu") == 0)
        {
          char nume_afisare[200];
          char final_afisare[200];
         
          bzero(&nume_afisare, sizeof(nume_afisare));
          bzero(&final_afisare, sizeof(final_afisare));
          read(tdL.cl, &nume_afisare, sizeof(nume_afisare));
       
          
          selectez_comentariu_mysql( nume_afisare,final_afisare);
     
          write(tdL.cl, &final_afisare, sizeof(final_afisare));
        }

        break;
        }
      }
    }
  }
} //void raspunde

//mysql pentru login
void login_mysql(char usr[], char pass[], int &nr_rows)
{

  MYSQL *con = mysql_init(NULL);

  if (con == NULL)
  {
    fprintf(stderr, "%s\n", mysql_error(con));
    exit(1);
  }

  mysql_real_connect(con, "den1.mysql5.gear.host", "topmusic", "Yx3Kv9~i?9C7", "topmusic", 0, NULL, 0);

  char a[200];
  char b[] = "' and password like binary '";
  char c[] = "'";
  pass[strlen(pass) - 1] = '\0';
  usr[strlen(usr)] = '\0';
  strcpy(a, "SELECT id_user FROM users WHERE username like binary '");
  strcat(a, usr);
  strcat(a, b);
  strcat(a, pass);
  strcat(a, c);

  if (mysql_query(con, a))
  {
    finish_with_error(con);
  }

  MYSQL_RES *result = mysql_store_result(con);

  if (result == NULL)
  {
    finish_with_error(con);
  }

  nr_rows = mysql_num_rows(result);
  mysql_free_result(result);
  mysql_close(con);
}

//mysql pentru REGISTER
void register_mysql(char usr[], char pass[])
{

  MYSQL *con = mysql_init(NULL);

  if (con == NULL)
  {
    fprintf(stderr, "%s\n", mysql_error(con));
    // exit(1);
  }

  mysql_real_connect(con, "den1.mysql5.gear.host", "topmusic", "Yx3Kv9~i?9C7", "topmusic", 0, NULL, 0);

  char a[200];
  char b[] = ",";
  char c[] = ",'1','user');";
  char d[] = "'";
  pass[strlen(pass) - 1] = '\0';
  usr[strlen(usr)] = '\0';
  strcpy(a, "INSERT INTO users (username,password,permisiune,grad)  VALUES('");
  strcat(a, usr);
  strcat(a, d);
  strcat(a, b);
  strcat(a, d);
  strcat(a, pass);
  strcat(a, d);
  strcat(a, c);

  if (mysql_query(con, a))
  {
    finish_with_error(con);
  }

  MYSQL_RES *result = mysql_store_result(con);

  mysql_free_result(result);
  mysql_close(con);
}
void addMusic_mysql(char nume_melodie[], char link_melodie[], char rock_melodie[], char dance_melodie[], char hiphop_melodie[])
{

  MYSQL *con = mysql_init(NULL);

  if (con == NULL)
  {
    fprintf(stderr, "%s\n", mysql_error(con));
    exit(1);
  }

  mysql_real_connect(con, "den1.mysql5.gear.host", "topmusic", "Yx3Kv9~i?9C7", "topmusic", 0, NULL, 0);

  char a[200];
  char virgula[] = ",";
  char c[] = ");";
  char apostrof[] = "'";

  nume_melodie[strlen(nume_melodie)] = '\0';
  link_melodie[strlen(link_melodie)] = '\0';
  rock_melodie[strlen(rock_melodie)] = '\0';
  dance_melodie[strlen(dance_melodie)] = '\0';
  hiphop_melodie[strlen(hiphop_melodie)] = '\0';
  strcpy(a, "INSERT INTO music (nume,link,rock,dance,hiphop)  VALUES('");
  strcat(a, nume_melodie);
  strcat(a, apostrof);
  strcat(a, virgula);
  strcat(a, apostrof);
  strcat(a, link_melodie);
  strcat(a, apostrof);
  strcat(a, virgula);
  strcat(a, apostrof);
  strcat(a, rock_melodie);
  strcat(a, apostrof);
  strcat(a, virgula);
  strcat(a, apostrof);
  strcat(a, dance_melodie);
  strcat(a, apostrof);
  strcat(a, virgula);
  strcat(a, apostrof);
  strcat(a, hiphop_melodie);
  strcat(a, apostrof);
  strcat(a, c);

  if (mysql_query(con, a))
  {
    finish_with_error(con);
  }

  MYSQL_RES *result = mysql_store_result(con);

  mysql_free_result(result);
}

void top_mysql(char top_melodie[])
{

  MYSQL *con = mysql_init(NULL);

  if (con == NULL)
  {
    fprintf(stderr, "%s\n", mysql_error(con));
    exit(1);
  }

  mysql_real_connect(con, "den1.mysql5.gear.host", "topmusic", "Yx3Kv9~i?9C7", "topmusic", 0, NULL, 0);

  char a[200];

  char voteaza[] = "'";
  voteaza[strlen(voteaza) - 1] = '\0';
  strcpy(a, "SELECT nume FROM music order by voturi desc");

  if (mysql_query(con, a))
  {
    finish_with_error(con);
  }

  MYSQL_RES *result = mysql_store_result(con);

  if (result == NULL)
  {
    finish_with_error(con);
  }

  int num_fields = mysql_num_fields(result);

  MYSQL_ROW row;

  while ((row = mysql_fetch_row(result)))
  {
    for (int i = 0; i < num_fields; i++)
    {
      strcat(top_melodie, row[i]);
      strcat(top_melodie, "@");
    }
  }

  mysql_free_result(result);
}

void topCategorie_mysql(char top_melodie[], char nume_categorie[])
{

  MYSQL *con = mysql_init(NULL);

  if (con == NULL)
  {
    fprintf(stderr, "%s\n", mysql_error(con));
    exit(1);
  }

  mysql_real_connect(con, "den1.mysql5.gear.host", "topmusic", "Yx3Kv9~i?9C7", "topmusic", 0, NULL, 0);

  char a[200];

  char voteaza[] = "'";
  char ordoneaza[] = " order by voturi desc ";
  //nume_categorie[strlen(nume_categorie) - 1] = '\0';
  strcpy(a, "SELECT nume FROM music where ");
  strcat(a, nume_categorie);
  strcat(a, "='y'");
  strcat(a, ordoneaza);
  if (mysql_query(con, a))
  {
    finish_with_error(con);
  }

  MYSQL_RES *result = mysql_store_result(con);

  if (result == NULL)
  {
    finish_with_error(con);
  }

  int num_fields = mysql_num_fields(result);

  MYSQL_ROW row;

  while ((row = mysql_fetch_row(result)))
  {
    for (int i = 0; i < num_fields; i++)
    {
      strcat(top_melodie, row[i]);
      strcat(top_melodie, "@");
    }
    printf("\n");
  }

  mysql_free_result(result);
  mysql_close(con);
}

void delete_mysql(char delete_melodie[])
{

  MYSQL *con = mysql_init(NULL);

  mysql_real_connect(con, "den1.mysql5.gear.host", "topmusic", "Yx3Kv9~i?9C7", "topmusic", 0, NULL, 0);

  char a[200];

  char sfarsit[] = "'";
  sfarsit[strlen(sfarsit) - 1] = '\0';
  strcpy(a, "delete from music where nume='");
  strcat(a, delete_melodie);
  strcat(a, "'");
  mysql_query(con, a);

  MYSQL_RES *result = mysql_store_result(con);

  mysql_free_result(result);
}

void admin_delete_mysql(char grad_delete[], char user[])
{

  MYSQL *con = mysql_init(NULL);

  if (con == NULL)
  {
    fprintf(stderr, "%s\n", mysql_error(con));
    exit(1);
  }

  mysql_real_connect(con, "den1.mysql5.gear.host", "topmusic", "Yx3Kv9~i?9C7", "topmusic", 0, NULL, 0);

  char a[200];

  //user[strlen(user) - 1] = '\0';
  //grad_delete[strlen(grad_delete) - 1] = '\0';
  strcpy(a, "SELECT grad FROM users where username='");
  strcat(a, user);
  strcat(a, "'");
  if (mysql_query(con, a))
  {
    finish_with_error(con);
  }

  MYSQL_RES *result = mysql_store_result(con);

  if (result == NULL)
  {
    finish_with_error(con);
  }

  int num_fields = mysql_num_fields(result);

  MYSQL_ROW row;

  while ((row = mysql_fetch_row(result)))
  {
    for (int i = 0; i < num_fields; i++)
    {
      strcat(grad_delete, row[i]);
    }
    printf("\n");
  }

  mysql_free_result(result);
  mysql_close(con);
}

void permisiune_0_mysql(char user[])
{

  MYSQL *con = mysql_init(NULL);

  if (con == NULL)
  {
    fprintf(stderr, "%s\n", mysql_error(con));
    exit(1);
  }

  mysql_real_connect(con, "den1.mysql5.gear.host", "topmusic", "Yx3Kv9~i?9C7", "topmusic", 0, NULL, 0);

  char a[200];

  strcpy(a, "UPDATE users SET permisiune=0 where username='");
  strcat(a, user);
  strcat(a, "'");
  if (mysql_query(con, a))
  {
    finish_with_error(con);
  }

  MYSQL_RES *result = mysql_store_result(con);

  mysql_free_result(result);
}

void permisiune_1_mysql(char user[])
{

  MYSQL *con = mysql_init(NULL);

  if (con == NULL)
  {
    fprintf(stderr, "%s\n", mysql_error(con));
    exit(1);
  }

  mysql_real_connect(con, "den1.mysql5.gear.host", "topmusic", "Yx3Kv9~i?9C7", "topmusic", 0, NULL, 0);

  char a[200];

  strcpy(a, "UPDATE users SET permisiune=1 where username='");
  strcat(a, user);
  strcat(a, "'");
  if (mysql_query(con, a))
  {
    finish_with_error(con);
  }

  MYSQL_RES *result = mysql_store_result(con);

  mysql_free_result(result);
}

void verificare_permisiune_mysql(char perm[], char user[])
{

  MYSQL *con = mysql_init(NULL);

  if (con == NULL)
  {
    fprintf(stderr, "%s\n", mysql_error(con));
    exit(1);
  }

  mysql_real_connect(con, "den1.mysql5.gear.host", "topmusic", "Yx3Kv9~i?9C7", "topmusic", 0, NULL, 0);

  char a[200];

  strcpy(a, "SELECT permisiune FROM users where username='");
  strcat(a, user);
  strcat(a, "'");
  if (mysql_query(con, a))
  {
    finish_with_error(con);
  }

  MYSQL_RES *result = mysql_store_result(con);

  if (result == NULL)
  {
    finish_with_error(con);
  }

  int num_fields = mysql_num_fields(result);

  MYSQL_ROW row;

  row = mysql_fetch_row(result);

  strcat(perm, row[0]);

  mysql_free_result(result);
  mysql_close(con);
  printf("Success verificare permisiune\n");
}

int scot_votare_mysql(char perm[])
{

  int vot;
  MYSQL *con = mysql_init(NULL);

  if (con == NULL)
  {
    fprintf(stderr, "%s\n", mysql_error(con));
    exit(1);
  }

  mysql_real_connect(con, "den1.mysql5.gear.host", "topmusic", "Yx3Kv9~i?9C7", "topmusic", 0, NULL, 0);

  char a[200];

  strcpy(a, "SELECT voturi FROM music where nume='");
  strcat(a, perm);
  strcat(a, "'");
  if (mysql_query(con, a))
  {
    finish_with_error(con);
  }

  MYSQL_RES *result = mysql_store_result(con);

  if (result == NULL)
  {
    finish_with_error(con);
  }

  int num_fields = mysql_num_fields(result);

  MYSQL_ROW row;

  row = mysql_fetch_row(result);

  mysql_free_result(result);
  mysql_close(con);
  printf("scot votare mysql %d\n", atoi(row[0]));
  return atoi(row[0]);
}
void adaug_votare_mysql(char nume_mel[], int vot)
{

  MYSQL *con = mysql_init(NULL);

  if (con == NULL)
  {
    fprintf(stderr, "%s\n", mysql_error(con));
    exit(1);
  }

  mysql_real_connect(con, "den1.mysql5.gear.host", "topmusic", "Yx3Kv9~i?9C7", "topmusic", 0, NULL, 0);

  char final[200];
  char a[200];
  sprintf(final, "%d", vot);

  strcpy(a, "UPDATE music SET voturi='");
  strcat(a, final);
  strcat(a, "' where nume='");
  strcat(a, nume_mel);
  strcat(a, "'");
  mysql_query(con, a);
}
void selectez_comentariu_mysql(char nume[],char final[])
{

  MYSQL *con = mysql_init(NULL);

  

  mysql_real_connect(con, "den1.mysql5.gear.host", "topmusic", "Yx3Kv9~i?9C7", "topmusic", 0, NULL, 0);
 
  
  char a[200];
  printf("Mda");
  strcpy(a, "SELECT comentarii from music where nume='");
  strcat(a, nume);
  strcat(a, "'");
  mysql_query(con, a);
  
  fflush(stdout);
   MYSQL_RES *result = mysql_store_result(con);
  int num_fields = mysql_num_fields(result);

  MYSQL_ROW row;

  while ((row = mysql_fetch_row(result)))
  {
    for (int i = 0; i < num_fields; i++)
    {
      strcat(final, row[i]);
      strcat(final," ");
    }
    printf("\n");
    
  }

 
  
}
void adaug_comentariu_mysql(char nume[], char com[])
{

  MYSQL *con = mysql_init(NULL);

  if (con == NULL)
  {
    fprintf(stderr, "%s\n", mysql_error(con));
    exit(1);
  }

  mysql_real_connect(con, "den1.mysql5.gear.host", "topmusic", "Yx3Kv9~i?9C7", "topmusic", 0, NULL, 0);

 
  char a[200];

  strcpy(a, "UPDATE music SET comentarii='");
  strcat(a, com);
  strcat(a, "' where nume='");
  strcat(a, nume);
  strcat(a,"'");
  mysql_query(con, a);

  
}