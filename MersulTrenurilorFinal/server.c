#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <sys/wait.h>
#include <strings.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#define PORT 2025
#define NMAX 256
#define MMAX 4096
char comanda[10][64];
sqlite3 *db;
char *err = 0;
const char delimitare[2] = " ";
int nr_parametri;
typedef struct
{
    char numar[10];
    char oras_plecare[50];
    char oras_sosire[50];
    char data_plecare[20];
    char ora_plecare[20];
    char data_sosire[20];
    char ora_sosire[20];
    char intarziere_plecare[10];
    char intarziere_sosire[10];
    char mai_devreme[10];
} tren;
tren trenuri[100];
int numar_trenuri;

typedef struct thData
{
    int idThread; // id-ul thread-ului tinut in evidenta de acest program
    int cl;       // descriptorul intors de accept
} thData;

void construire_comanda(char msg[256])
{
    char *p;
    nr_parametri = 1;
    int i;
    i = 1;
    p = strtok(msg, delimitare);
    while (p != NULL)
    {
        nr_parametri++;
        strcpy(comanda[i++], p);
        p = strtok(NULL, delimitare);
    }
    strtok(comanda[nr_parametri - 1], "\n");
}
//=======================================================XML======================================================//
void parse_xml(char *filename)
{
    xmlDoc *doc = NULL;
    xmlNode *root_element = NULL;

    // Incarcam fisierul XML in memorie
    doc = xmlReadFile(filename, NULL, 0);
    if (doc == NULL)
    {
        printf("Eroare la incarcarea fisierului XML '%s'\n", filename);
        return;
    }

    // Obtinem radacina documentului XML
    root_element = xmlDocGetRootElement(doc);
    if (root_element == NULL)
    {
        printf("Fisierul XML nu contine elemente\n");
        xmlFreeDoc(doc);
        return;
    }

    // Parcurgem fiecare nod "tren" din fisierul XML
    xmlNode *cur_node = NULL;
    int i = 0;
    for (cur_node = root_element->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE)
        {
            if (strcmp((char *)cur_node->name, "tren") == 0)
            {
                xmlAttr *attr = NULL;
                // Extragem atributul "numar" din nodul "tren"
                for (attr = cur_node->properties; attr; attr = attr->next)
                {
                    if (strcmp((char *)attr->name, "numar") == 0)
                    {
                        strcpy(trenuri[i].numar, (char *)attr->children->content);
                        break;
                    }
                }
                // Parcurgem copiii nodului "tren" pentru a extrage trenuri[i]rmatiile
                xmlNode *child = NULL;
                for (child = cur_node->children; child; child = child->next)
                {
                    if (child->type == XML_ELEMENT_NODE)
                    {
                        // Extragem trenuri[i]rmatiile din fiecare copil al nodului "tren"
                        if (strcmp((char *)child->name, "oras_plecare") == 0)
                        {
                            strcpy(trenuri[i].oras_plecare, (char *)child->children->content);
                        }
                        else if (strcmp((char *)child->name, "oras_sosire") == 0)
                        {
                            strcpy(trenuri[i].oras_sosire, (char *)child->children->content);
                        }
                        else if (strcmp((char *)child->name, "data_plecare") == 0)
                        {
                            strcpy(trenuri[i].data_plecare, (char *)child->children->content);
                        }
                        else if (strcmp((char *)child->name, "ora_plecare") == 0)
                        {
                            strcpy(trenuri[i].ora_plecare, (char *)child->children->content);
                        }
                        else if (strcmp((char *)child->name, "data_sosire") == 0)
                        {
                            strcpy(trenuri[i].data_sosire, (char *)child->children->content);
                        }
                        else if (strcmp((char *)child->name, "ora_sosire") == 0)
                        {
                            strcpy(trenuri[i].ora_sosire, (char *)child->children->content);
                        }
                        else if (strcmp((char *)child->name, "intarziere_plecare") == 0)
                        {
                            strcpy(trenuri[i].intarziere_plecare, (char *)child->children->content);
                        }
                        else if (strcmp((char *)child->name, "intarziere_sosire") == 0)
                        {
                            strcpy(trenuri[i].intarziere_sosire, (char *)child->children->content);
                        }
                        else if (strcmp((char *)child->name, "mai_devreme") == 0)
                        {
                            strcpy(trenuri[i].mai_devreme, (char *)child->children->content);
                        }
                    }
                }
                // Afisam trenuri[i]rmatiile extrase pentru fiecare tren
            }
            i++;
        }
    }
    numar_trenuri = i;
    // Eliberam memoria alocata pentru documentul XML
    xmlFreeDoc(doc);
}

void modifica_struct(char *numar, char *field, char *value) // functie care modifica un anumit camp din structura unui tren
{

    for (int i = 0; i < numar_trenuri; i++)
        if (strcmp((char *)trenuri[i].numar, numar) == 0)
        {
            if (strcmp("oras_plecare", field) == 0)
                strcpy(trenuri[i].oras_plecare, value);
            else if (strcmp("oras_sosire", field) == 0)
                strcpy(trenuri[i].oras_sosire, value);
            else if (strcmp("data_plecare", field) == 0)
                strcpy(trenuri[i].data_plecare, value);
            else if (strcmp("data_sosire", field) == 0)
                strcpy(trenuri[i].data_sosire, value);
            else if (strcmp("ora_plecare", field) == 0)
                strcpy(trenuri[i].ora_plecare, value);
            else if (strcmp("ora_sosire", field) == 0)
                strcpy(trenuri[i].ora_sosire, value);
            else if (strcmp("intarziere_plecare", field) == 0)
                strcpy(trenuri[i].intarziere_plecare, value);
            else if (strcmp("intarziere_sosire", field) == 0)
                strcpy(trenuri[i].intarziere_sosire, value);
            else if (strcmp("mai_devreme", field) == 0)
                strcpy(trenuri[i].mai_devreme, value);
            break;
        }
}
// Functie pentru a modifica fisierul XML
void modify_xml(char *numar, char *field, char *value, char mesaj[NMAX], int *ok)
{
    // Incarcam fisierul XML in memorie
    xmlDoc *doc = xmlReadFile("trenuri.xml", NULL, 0);
    if (doc == NULL)
    {
        printf("Eroare la incarcarea fisierului XML '%s'\n", "trenuri.xml");
        return;
    }
    int g = 0;
    // Obtinem radacina documentului XML
    xmlNode *root_element = xmlDocGetRootElement(doc);
    if (root_element == NULL)
    {
        printf("Fisierul XML nu contine elemente\n");
        xmlFreeDoc(doc);
        return;
    }

    // Parcurgem fiecare nod "tren" din fisierul XML
    xmlNode *cur_node = NULL;
    for (cur_node = root_element->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE && strcmp((char *)cur_node->name, "tren") == 0)
        {
            xmlAttr *attr = NULL;
            // Cautam nodul "tren" care are atributul "numar" egal cu numarul trenului
            for (attr = cur_node->properties; attr; attr = attr->next)
            {
                if (strcmp((char *)attr->name, "numar") == 0 && strcmp((char *)attr->children->content, numar) == 0)
                {
                    break;
                }
            }
            // Daca am gasit nodul, modificam informatiile
            if (attr != NULL)
            {
                g = 1;
                xmlNode *child = NULL;
                for (child = cur_node->children; child; child = child->next)
                {
                    if (child->type == XML_ELEMENT_NODE && strcmp((char *)child->name, field) == 0)
                    {
                        xmlNodeSetContent(child, (xmlChar *)value);
                        modifica_struct(numar, field, value);
                        break;
                    }
                }
            }
        }
    }
    if (g == 0)
    {
        strcpy(mesaj, "[server]Tren inexistent!\n");
        *ok = 0;
    }
    else
        *ok = 1;
    // Salvam modificarile in fisierul XML
    xmlSaveFormatFileEnc("trenuri.xml", doc, "UTF-8", 1);

    // Eliberam memoria alocata pentru documentul XML
    xmlFreeDoc(doc);
}
void extract_field(char *filename, char *numar, char *field, char *sir)
{
    char value[100]; // Buffer pentru a stoca valoarea câmpului
    value[0] = '\0'; // Initializam buffer-ul cu sirul de caractere vid

    // Incarcam fisierul XML in memorie
    xmlDoc *doc = xmlReadFile(filename, NULL, 0);
    if (doc == NULL)
    {
        printf("Eroare la incarcarea fisierului XML '%s'\n", filename);
        exit(1);
    }

    // Obtinem radacina documentului XML
    xmlNode *root_element = xmlDocGetRootElement(doc);
    if (root_element == NULL)
    {
        printf("Fisierul XML nu contine elemente\n");
        xmlFreeDoc(doc);
        exit(1);
    }

    // Parcurgem fiecare nod "tren" din fisierul XML
    xmlNode *cur_node = NULL;
    for (cur_node = root_element->children; cur_node; cur_node = cur_node->next)
    {
        if (cur_node->type == XML_ELEMENT_NODE && strcmp((char *)cur_node->name, "tren") == 0)
        {
            xmlAttr *attr = NULL;
            // Cautam nodul "tren" care are atributul "numar" egal cu numarul trenului
            for (attr = cur_node->properties; attr; attr = attr->next)
            {
                if (strcmp((char *)attr->name, "numar") == 0 && strcmp((char *)attr->children->content, numar) == 0)
                {
                    break;
                }
            }
            // Daca am gasit nodul, extragem valoarea câmpului
            if (attr != NULL)
            {
                xmlNode *child = NULL;
                for (child = cur_node->children; child; child = child->next)
                {
                    if (child->type == XML_ELEMENT_NODE && strcmp((char *)child->name, field) == 0)
                    {
                        strcpy(value, (char *)xmlNodeGetContent(child));
                        break;
                    }
                }
            }
        }
    }
    // Eliberam memoria alocata pentru documentul XML
    xmlFreeDoc(doc);
    strcpy(sir, value);
}
//=======================================================Baza de date======================================================//
void setDataBase()
{
    int rc = sqlite3_open("users.db", &db);
    if (rc != SQLITE_OK)
    {
        printf("Eroare\n");
    }
    sqlite3_exec(db, "CREATE TABLE IF NOT EXISTS Users(ID INTEGER PRIMARY KEY, Nume TEXT,Parola TEXT,Status INT);", 0, 0, &err);
    sqlite3_exec(db, "Update Users SET Status=0;", 0, 0, &err);
}
void ROpenBase()
{
    int rc = sqlite3_open("users.db", &db);
    if (rc != SQLITE_OK)
    {
        printf("Eroare\n");
    }
}
//=======================================================Logare======================================================//
void codificare(char parola[256])
{
    int i, j;
    char alfabet[63] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    char cuvant[strlen(parola)];
    cuvant[0] = '\0';
    for (i = 0; i < strlen(parola); i++)
    {
        int g = 0;
        for (j = 0; j < strlen(alfabet) && g == 0; j++)
            if (parola[i] == alfabet[j])
                g = 1;
        j--;
        if (parola[i] >= 'a' && parola[i] <= 'z')
        {
            cuvant[i] = alfabet[(51 - j) + 26];
        }
        else if (parola[i] >= 'A' && parola[i] <= 'Z')
        {
            cuvant[i] = alfabet[25 - j];
        }
        else if (parola[i] >= '0' && parola[i] <= '9')
        {
            cuvant[i] = alfabet[(61 - j) + 52];
        }
    }
    cuvant[strlen(parola)] = '\0';
    strcpy(parola, cuvant);
}
int exista(char nume[NMAX])
{
    sqlite3_stmt *stmt;
    char sql[NMAX];
    strcpy(sql, "SELECT Id FROM Users WHERE Nume='");
    strcat(sql, nume);
    strcat(sql, "';");
    sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    int id;
    sqlite3_step(stmt);
    id = sqlite3_column_int(stmt, 0);
    if (id != 0)
    {
        sqlite3_finalize(stmt);
        return 1;
    }
    else
    {
        sqlite3_finalize(stmt);
        return 0;
    }
}
int userLogatClientDiferit(char nume[NMAX])
{
    sqlite3_stmt *stmt;
    char sql[NMAX];
    strcpy(sql, "SELECT Status FROM Users WHERE Nume='");
    strcat(sql, nume);
    strcat(sql, "';");
    sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    sqlite3_step(stmt);
    int id;
    id = sqlite3_column_int(stmt, 0);
    if (id != 0)
    {
        sqlite3_finalize(stmt);
        return 1;
    }
    else
    {
        sqlite3_finalize(stmt);
        return 0;
    }
}
void adauga_user(char nume[NMAX], char parola[NMAX])
{
    char sql[256];
    char *err = 0;
    sql[0] = '\0';
    strcpy(sql, "INSERT INTO Users(Nume,Parola) VALUES('");
    strcat(sql, nume);
    strcat(sql, "','");
    strcat(sql, parola);
    strcat(sql, "');");
    printf("%s\n", sql);
    sqlite3_exec(db, sql, 0, 0, &err);
    sqlite3_close_v2(db);
    ROpenBase();
}
void schimbaStatus(char nume[NMAX], char status[2])
{
    char sql[256];
    bzero(sql, 256);
    strcpy(sql, "UPDATE Users SET Status=");
    strcat(sql, status);
    strcat(sql, "  WHERE Nume='");
    strcat(sql, nume);
    strcat(sql, "';");
    sqlite3_exec(db, sql, 0, 0, &err);
    sqlite3_close_v2(db);
    ROpenBase();
}
int verificareParola(char nume[NMAX], char parola[NMAX])
{
    sqlite3_stmt *stmt;
    char sql[NMAX];
    strcpy(sql, "SELECT Parola FROM Users WHERE Nume='");
    strcat(sql, nume);
    strcat(sql, "';");
    sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    sqlite3_step(stmt);
    char pass[NMAX];
    char *msg = (char *)sqlite3_column_text(stmt, 0);
    codificare(msg);

    printf("Parola = %s\n", msg);

    if (strcmp(parola, msg) == 0)
    {
        sqlite3_finalize(stmt);
        return 1;
    }
    else
    {
        sqlite3_finalize(stmt);
        return 0;
    }
}
void logare(char mesaj[MMAX], int *indicator_logare, char username[NMAX])
{
    char nume[NMAX];
    char parola[NMAX];
    strcpy(nume, comanda[2]);
    strcpy(parola, comanda[3]);
    if (exista(nume) == 1)
    {
        if (userLogatClientDiferit(nume) == 1)
            strcpy(mesaj, "[server]Acest user este deja logat intr-un alt client!");
        else
        {
            if (verificareParola(nume, parola) == 1)
            {
                schimbaStatus(nume, "1");
                strcpy(mesaj, "\e[1;32m[server]V-ati logat cu succes cu username-ul: ");
                strcat(mesaj, nume);
                strcat(mesaj, " \e[0m");
                strcpy(username, nume);
                *indicator_logare = 1;
            }
            else
            {
                strcpy(mesaj, "[server]Parola incorecta!");
            }
        }
    }
    else
    {
        strcpy(mesaj, "[server]Utilizator inexistent!");
    }
}
void sign_in(char mesaj[MMAX], int *indicator_logare, char username[NMAX])
{
    char nume[NMAX];
    char parola[NMAX];
    bzero(nume, NMAX);
    bzero(parola, NMAX);
    strcpy(nume, comanda[2]);
    strcpy(parola, comanda[3]);
    if (exista(nume) == 0)
    {
        codificare(parola);
        adauga_user(nume, parola);
        schimbaStatus(nume, "1");
        *indicator_logare = 1;
        strcpy(mesaj, "[server]V-ati inregistrat cu succes cu username-ul: ");
        strcat(mesaj, nume);
        strcpy(username, nume);
        sqlite3_close_v2(db);
        ROpenBase();
    }
    else
    {
        strcpy(mesaj, "[server]Utilizator existent! Introduceti un alt nume de utilizator");
    }
}
void logout(char mesaj[MMAX], int *indicator_logare, char username[NMAX])
{
    *indicator_logare = 0;
    schimbaStatus(username, "0");
}
//===================================================================================================================//
//=======================================================Trenuri=====================================================//
void getDate(char *date)
{
    struct timeval tv;
    time_t t;
    struct tm *info;
    char buffer[64];
    gettimeofday(&tv, NULL);
    t = tv.tv_sec;
    info = localtime(&t);
    strftime(buffer, sizeof buffer, "%d.%m.%Y", info);
    strcpy(date, buffer);
}
void getTime(char *date)
{
    struct timeval tv;
    time_t t;
    struct tm *info;
    char buffer[64];
    gettimeofday(&tv, NULL);
    t = tv.tv_sec;
    info = localtime(&t);
    strftime(buffer, sizeof buffer, "%H:%M", info);
    strcpy(date, buffer);
}
void getTimeAdunare(char *date, int intarziere)
{
    char buffer[3];
    int ora, minute;
    char *p;
    p = strtok(date, ":");
    ora = atoi(p);
    p = strtok(NULL, ":");
    minute = atoi(p);
    struct tm timeStruct;
    timeStruct.tm_hour = ora;
    timeStruct.tm_min = minute;
    timeStruct.tm_sec = 0;

    // Verifica daca intarzierea este negativa
    if (intarziere < 0) {
        // Scade intarzierea din minute
        timeStruct.tm_min -= abs(intarziere);
    } else {
        // Adauga intarzierea la minute
        timeStruct.tm_min += intarziere;
    }

    // Verifica daca ora este mai mare sau egala cu 24
    if (timeStruct.tm_hour >= 24) {
        // Scade o zi din data curenta
        timeStruct.tm_mday--;
        // Scade 24 de la ora
        timeStruct.tm_hour -= 24;
    }
    timeStruct.tm_hour += timeStruct.tm_min / 60;
    timeStruct.tm_min %= 60;
    timeStruct.tm_hour %= 24;
    time_t newTime = mktime(&timeStruct);
    strftime(date, sizeof(date), "%H:%M", &timeStruct);
}

int comparaOra(char date1[20], char date2[20])
{
    char tren[20];
    char local[20];
    strcpy(tren, date1);
    strcpy(local, date2);
    char buffer[3];
    int oraTren, minuteTren, oraLocala, minuteLocala;
    char *p1, *p2;
    p1 = strtok(tren, ":");
    oraTren = atoi(p1);
    p1 = strtok(NULL, ":");
    minuteTren = atoi(p1);
    p2 = strtok(local, ":");
    oraLocala = atoi(p2);
    p2 = strtok(NULL, ":");
    minuteLocala = atoi(p2);
    struct tm timeStructTren, timeStructLocal;
    timeStructTren.tm_hour = oraTren;
    timeStructTren.tm_min = minuteTren;
    timeStructLocal.tm_hour = oraLocala;
    timeStructLocal.tm_min = minuteLocala;
    if ((oraTren == oraLocala && minuteTren >= minuteLocala) || (oraTren == (oraLocala+1) && minuteTren < minuteLocala))
        return 1;
    else
        return 0;
}
void extrageTag(char *xml, char *tag, char *cuvant)
{
    char *p;
    p = strstr(xml, tag);
    int i, j, start, end;
    for (i = 1; i < strlen(p); i++)
    {
        if (p[i] == '>')
            start = i + 1;
        if (p[i] == '<')
            break;
    }
    end = i - 1;
    j = 0;
    cuvant[0] = '\0';
    for (i = start; i <= end; i++)
    {
        cuvant[j] = p[i];
        j++;
    }
    if (strlen(cuvant) == 3)
        cuvant[1] = '\0';
    else
        cuvant[strlen(cuvant)] = '\0';
}
void getPlecari(char mesaj[NMAX])
{
    int i, g = 0;
    char time[NMAX];
    char date[NMAX];
    char msg[MMAX];
    bzero(msg, MMAX);
    bzero(mesaj, MMAX);
    getTime(time);
    getDate(date);
    sprintf(mesaj, "\e[1;32m================Trenurile care pleaca in aceasta ora %s================\e[0m \n", time);
    // strcpy(mesaj, "id|Plecare|Sosire|Data_plecare|Ora_Plecare|Data_Sosire|Ora_sosire|Intarziere_Plecare|Intarziere_Sosire|Sosire_Mai_Devreme\n");
    for (i = 0; i < numar_trenuri; i++)
    {
        getTime(time);
        if (comparaOra(trenuri[i].ora_plecare, time) == 1 && strcmp(trenuri[i].data_plecare, date) == 0)
        {
            g = 1;
            sprintf(msg, "Id tren : %s\nOras de plecare: %s\nOras de sosire: %s\nData plecarii : %s\nOra plecarii : %s\nData sosirii : %s\nOra sosirii : %s\nIntarziere la plecare de : %s minute\nIntarziere la sosire de : %s minute\nSosire mai devreme cu : %s minute\n",
                    trenuri[i].numar, trenuri[i].oras_plecare, trenuri[i].oras_sosire, trenuri[i].data_plecare, trenuri[i].ora_plecare,
                    trenuri[i].data_sosire, trenuri[i].ora_sosire, trenuri[i].intarziere_plecare, trenuri[i].intarziere_sosire, trenuri[i].mai_devreme);
            strcat(mesaj, msg);
            if (atoi(trenuri[i].intarziere_plecare) > 0)
            {
                strcpy(time, trenuri[i].ora_plecare);
                getTimeAdunare(time, atoi(trenuri[i].intarziere_plecare));
                sprintf(msg, "\e[1;31mTrenul are o intarziere de %s minute si va pleca la %s\e[0m \n", trenuri[i].intarziere_plecare, time);
                strcat(mesaj, msg);
            }
            else
            {
                strcat(mesaj, "\e[1;32mTrenul pleaca la timp\e[0m \n");
            }

            strcat(mesaj, "\e[1;32m=======================================================================\e[0m \n");
        }
    }
    if (g == 0)
    {
        sprintf(msg, "\e[1;31m[server]Nu este niciun tren care sa plece in urmatoarea ora, %s\e[0m", time);
        strcpy(mesaj, msg);
    }
}
void getSosiri(char mesaj[NMAX])
{
    int i, g = 0;
    char time[NMAX];
    char date[NMAX];
    char msg[MMAX];
    bzero(msg, MMAX);
    bzero(mesaj, MMAX);
    getTime(time);
    getDate(date);
    sprintf(mesaj, "\e[1;32m================Trenurile care sosesc in aceasta ora %s================\e[0m \n", time);
    for (i = 0; i < numar_trenuri; i++)
    {
        getTime(time);
        if (comparaOra(trenuri[i].ora_sosire, time) == 1 && strcmp(trenuri[i].data_sosire, date) == 0)
        {   
            g = 1;
            sprintf(msg, "Id tren : %s\nOras de plecare: %s\nOras de sosire: %s\nData plecarii : %s\nOra plecarii : %s\nData sosirii : %s\nOra sosirii : %s\nIntarziere la plecare de : %s minute\nIntarziere la sosire de : %s minute\nSosire mai devreme cu : %s minute\n",
                    trenuri[i].numar, trenuri[i].oras_plecare, trenuri[i].oras_sosire, trenuri[i].data_plecare, trenuri[i].ora_plecare,
                    trenuri[i].data_sosire, trenuri[i].ora_sosire, trenuri[i].intarziere_plecare, trenuri[i].intarziere_sosire, trenuri[i].mai_devreme);
            strcat(mesaj, msg);
            if ((atoi(trenuri[i].intarziere_plecare) + atoi(trenuri[i].intarziere_sosire) - atoi(trenuri[i].mai_devreme)) == 0)
            {
                strcpy(msg, "\e[1;32mTrenul va sosi la timp\e[0m \n");
                strcat(mesaj, msg);
            }
            else if ((atoi(trenuri[i].intarziere_plecare) + atoi(trenuri[i].intarziere_sosire) - atoi(trenuri[i].mai_devreme)) > 0)
            {
                strcpy(time, trenuri[i].ora_sosire);
                getTimeAdunare(time, (atoi(trenuri[i].intarziere_plecare) + atoi(trenuri[i].intarziere_sosire) - atoi(trenuri[i].mai_devreme)));
                printf("Timp = %s\n", time);
                sprintf(msg, "\e[1;31mTrenul va ajunge cu o intarziere de %d minute la %s\e[0m \n", (atoi(trenuri[i].intarziere_plecare) + atoi(trenuri[i].intarziere_sosire) - atoi(trenuri[i].mai_devreme)), time);
                strcat(mesaj, msg);
            }
            else
            {
                strcpy(time, trenuri[i].ora_sosire);
                getTimeAdunare(time, atoi(trenuri[i].intarziere_sosire) + atoi(trenuri[i].intarziere_plecare) - atoi(trenuri[i].mai_devreme));
                sprintf(msg, "\e[1;31mTrenul va ajunge mai devreme cu %d minute la %s\e[0m \n", atoi(trenuri[i].mai_devreme) - (atoi(trenuri[i].intarziere_sosire) + atoi(trenuri[i].intarziere_plecare)), time);
                strcat(mesaj, msg);
            }

            strcat(mesaj, "\e[1;32m=======================================================================\e[0m \n");
        }
    }
    if (g == 0)
    {
        sprintf(msg, "\e[1;31m[server]Nu este niciun tren care sa soseasca in urmatoarea ora, %s\e[0m", time);
        strcpy(mesaj, msg);
    }
}
void getInfoAstazi(char mesaj[MMAX])
{
    int i, g = 0;
    char date[NMAX];
    char msg[MMAX];
    bzero(mesaj, MMAX);
    bzero(msg, MMAX);
    getDate(date);
    sprintf(mesaj, "\e[1;32m================Mersul trenurilor de astazi %s================\e[0m \n", date);
    for (i = 0; i < numar_trenuri; i++)
    {
        if (strcmp(trenuri[i].data_plecare, date) == 0)
        {
            g = 1;
            sprintf(msg, "Id tren : %s\nOras de plecare: %s\nOras de sosire: %s\nData plecarii : %s\nOra plecarii : %s\nData sosirii : %s\nOra sosirii : %s\nIntarziere la plecare de : %s minute\nIntarziere la sosire de : %s minute\nSosire mai devreme cu : %s minute\n",
                    trenuri[i].numar, trenuri[i].oras_plecare, trenuri[i].oras_sosire, trenuri[i].data_plecare, trenuri[i].ora_plecare,
                    trenuri[i].data_sosire, trenuri[i].ora_sosire, trenuri[i].intarziere_plecare, trenuri[i].intarziere_sosire, trenuri[i].mai_devreme);
            strcat(mesaj, msg);
            strcat(mesaj, "\e[1;32m=====================================================================\e[0m \n");
        }
    }
    if (g == 0)
    {
        sprintf(msg, "\e[1;31m[[server]Nu este niciun tren care sa plece astazi, %s\e[0m", date);
        strcpy(mesaj, msg);
    }
}
void setIntarzierePlecare(char mesaj[MMAX])
{
    int ok = 0;
    char *id = comanda[2];
    char *value = comanda[3];
    printf("id = %s  value = %s\n", id, value);
    modify_xml(id, "intarziere_plecare", value, mesaj, &ok);
    if (ok == 1)
    {

        sprintf(mesaj, "\e[1;32m[server]Modificare facuta cu succes, trenul cu id-ul %s are acum o intarziere la plecare de %s minute.\e[0m\n", id, value);
    }
}
void setIntarziereSosire(char mesaj[MMAX])
{
    int ok = 0;
    char *id = comanda[2];
    char *value = comanda[3];
    printf("id = %s  value = %s\n", id, value);
    modify_xml(id, "intarziere_sosire", value, mesaj, &ok);
    if (ok == 1)
    {
        sprintf(mesaj, "\e[1;32m[server]Modificare facuta cu succes, trenul cu id-ul %s are acum o intarziere la sosire de %s minute.\e[0m\n", id, value);
    }
}
void setSosireDevreme(char mesaj[MMAX])
{
    int ok = 0;
    char *id = comanda[2];
    char *value = comanda[3];
    printf("id = %s  value = %s\n", id, value);
    modify_xml(id, "mai_devreme", value, mesaj, &ok);
    if (ok == 1)
    {
        sprintf(mesaj, "\e[1;32m[server]Modificare facuta cu succes, trenul cu id-ul %s ajunge mai devreme %s minute.\e[0m\n", id, value);
    };
}
void getOrasPlecare(char mesaj[MMAX])
{
    int g = 0;
    char oras[NMAX];
    strcpy(oras, comanda[2]);
    char msg[MMAX];
    bzero(msg, MMAX);
    sprintf(mesaj, "\e[1;32m================Mersul trenurilor care pleaca din %s================\e[0m \n", oras);
    for (int i = 0; i < numar_trenuri; i++)
        if (strcmp(trenuri[i].oras_plecare, oras) == 0)
        {
            g = 1;
            sprintf(msg, "Id tren : %s\nOras de plecare: %s\nOras de sosire: %s\nData plecarii : %s\nOra plecarii : %s\nData sosirii : %s\nOra sosirii : %s\nIntarziere la plecare de : %s minute\nIntarziere la sosire de : %s minute\nSosire mai devreme cu : %s minute\n",
                    trenuri[i].numar, trenuri[i].oras_plecare, trenuri[i].oras_sosire, trenuri[i].data_plecare, trenuri[i].ora_plecare,
                    trenuri[i].data_sosire, trenuri[i].ora_sosire, trenuri[i].intarziere_plecare, trenuri[i].intarziere_sosire, trenuri[i].mai_devreme);
            strcat(mesaj, msg);
            strcat(mesaj, "\e[1;32m=====================================================================\e[0m \n");
        }
    if (g == 0)
    {
        sprintf(msg, "\e[1;31m[server]Nu este niciun tren care sa plece din %s\e[0m", oras);
        strcpy(mesaj, msg);
    }
}
void getOrasSosire(char mesaj[MMAX])
{
    int g = 0;
    char oras[NMAX];
    strcpy(oras, comanda[2]);
    char msg[MMAX];
    bzero(msg, MMAX);
    sprintf(mesaj, "\e[1;32m================Mersul trenurilor care sosesc in %s================\e[0m \n", oras);
    for (int i = 0; i < numar_trenuri; i++)
        if (strcmp(trenuri[i].oras_sosire, oras) == 0)
        {
            g = 1;
            sprintf(msg, "Id tren : %s\nOras de plecare: %s\nOras de sosire: %s\nData plecarii : %s\nOra plecarii : %s\nData sosirii : %s\nOra sosirii : %s\nIntarziere la plecare de : %s minute\nIntarziere la sosire de : %s minute\nSosire mai devreme cu : %s minute\n",
                    trenuri[i].numar, trenuri[i].oras_plecare, trenuri[i].oras_sosire, trenuri[i].data_plecare, trenuri[i].ora_plecare,
                    trenuri[i].data_sosire, trenuri[i].ora_sosire, trenuri[i].intarziere_plecare, trenuri[i].intarziere_sosire, trenuri[i].mai_devreme);
            strcat(mesaj, msg);
            strcat(mesaj, "\e[1;32m=====================================================================\e[0m \n");
        }
    if (g == 0)
    {
        sprintf(msg, "\e[1;31m[[server]Nu este niciun tren care sa soseasca in %s\e[0m", oras);
        strcpy(mesaj, msg);
    }
}
void getStatii(char mesaj[MMAX])
{
    int g = 0;
    char orase[NMAX];
    char camp[NMAX];
    char msg[MMAX];
    bzero(msg, MMAX);
    bzero(orase, NMAX);
    strcpy(camp, comanda[2]);
    if (strcmp(camp, "plecare") == 0)
    {
        sprintf(mesaj, "\e[1;32m=============== Statiile din care pleaca trenuri ================\e[0m \n");
        for (int i = 0; i < numar_trenuri; i++)
            if (!strstr(orase, trenuri[i].oras_plecare))
            {
                strcat(orase, trenuri[i].oras_plecare);
                sprintf(msg, "Statie plecare %s \n", trenuri[i].oras_plecare);
                strcat(mesaj, msg);
            }
    }
    else if (strcmp(camp, "sosire") == 0)
    {
        sprintf(mesaj, "\e[1;32m================ Statiile in care sosesc trenuri ================\e[0m \n");
        for (int i = 0; i < numar_trenuri; i++)
            if (!strstr(orase, trenuri[i].oras_sosire))
            {
                strcat(orase, trenuri[i].oras_sosire);
                sprintf(msg, "Statie sosire %s \n", trenuri[i].oras_sosire);
                strcat(mesaj, msg);
            }
    }
    else
    {
        strcpy(mesaj, "\e[1;31m===================== Camp de cautare gresit ====================\e[0m \n");
    }
}
void ruteTrenuri(char mesaj[MMAX])
{
    int g = 0;
    char plecare[NMAX];
    char sosire[NMAX];
    char msg[MMAX];
    char time[NMAX];
    bzero(time,NMAX);
    bzero(msg, MMAX);
    bzero(plecare, NMAX);
    bzero(sosire, NMAX);
    strcpy(plecare, comanda[2]);
    strcpy(sosire, comanda[3]);
    sprintf(msg, "\e[1;32m========= Trenurile care pleaca din %s si sosesc in %s ==========\e[0m \n", plecare, sosire);
    strcpy(mesaj,msg);
    for (int i = 0; i < numar_trenuri; i++)
        if (strcmp(trenuri[i].oras_plecare, plecare) == 0 && strcmp(trenuri[i].oras_sosire, sosire) == 0)
        {
            getTime(time);
            g = 1;
            sprintf(msg, "Id tren : %s\nOras de plecare: %s\nOras de sosire: %s\nData plecarii : %s\nOra plecarii : %s\nData sosirii : %s\nOra sosirii : %s\nIntarziere la plecare de : %s minute\nIntarziere la sosire de : %s minute\nSosire mai devreme cu : %s minute\n",
                    trenuri[i].numar, trenuri[i].oras_plecare, trenuri[i].oras_sosire, trenuri[i].data_plecare, trenuri[i].ora_plecare,
                    trenuri[i].data_sosire, trenuri[i].ora_sosire, trenuri[i].intarziere_plecare, trenuri[i].intarziere_sosire, trenuri[i].mai_devreme);
            strcat(mesaj, msg);
            if ((atoi(trenuri[i].intarziere_plecare) + atoi(trenuri[i].intarziere_sosire) - atoi(trenuri[i].mai_devreme)) == 0)
            {
                strcpy(msg, "\e[1;32mTrenul va sosi la timp\e[0m \n");
                strcat(mesaj, msg);
            }
            else if ((atoi(trenuri[i].intarziere_plecare) + atoi(trenuri[i].intarziere_sosire) - atoi(trenuri[i].mai_devreme)) > 0)
            {
                strcpy(time, trenuri[i].ora_sosire);
                getTimeAdunare(time, (atoi(trenuri[i].intarziere_plecare) + atoi(trenuri[i].intarziere_sosire) - atoi(trenuri[i].mai_devreme)));
                sprintf(msg, "\e[1;31mTrenul va ajunge cu o intarziere de %d minute la %s\e[0m \n", (atoi(trenuri[i].intarziere_plecare) + atoi(trenuri[i].intarziere_sosire) - atoi(trenuri[i].mai_devreme)), time);
                strcat(mesaj, msg);
            }
            else
            {
                strcpy(time, trenuri[i].ora_sosire);
                getTimeAdunare(time, atoi(trenuri[i].intarziere_sosire) + atoi(trenuri[i].intarziere_plecare) - atoi(trenuri[i].mai_devreme));
                sprintf(msg, "\e[1;31mTrenul va ajunge mai devreme cu %d minute la %s\e[0m \n", atoi(trenuri[i].mai_devreme) - (atoi(trenuri[i].intarziere_sosire) + atoi(trenuri[i].intarziere_plecare)), time);
                strcat(mesaj, msg);
            }
            strcat(mesaj, "\e[1;32m=====================================================================\e[0m \n");
        }
    if (g == 0)
    {
        sprintf(msg, "\e[1;31m[server]Nu este niciun tren care sa plece din %s si sa soseasca in %s.\e[0m", plecare, sosire);
        strcpy(mesaj, msg);
    }
}
void getTrenById(char mesaj[MMAX])
{
    int g = 0;
    char time[NMAX];
    char id[NMAX];
    char msg[MMAX];
    bzero(msg, MMAX);
    bzero(id, NMAX);
    bzero(time,NMAX);
    strcpy(id, comanda[2]);
    sprintf(msg, "\e[1;32m===================== Trenurile cu id-ul %s======================\e[0m \n", id);
    strcpy(mesaj,msg);
    for (int i = 0; i < numar_trenuri; i++)
        if (strcmp(trenuri[i].numar, id) == 0)
        {
            getTime(time);
            g = 1;
            sprintf(msg, "Id tren : %s\nOras de plecare: %s\nOras de sosire: %s\nData plecarii : %s\nOra plecarii : %s\nData sosirii : %s\nOra sosirii : %s\nIntarziere la plecare de : %s minute\nIntarziere la sosire de : %s minute\nSosire mai devreme cu : %s minute\n",
                    trenuri[i].numar, trenuri[i].oras_plecare, trenuri[i].oras_sosire, trenuri[i].data_plecare, trenuri[i].ora_plecare,
                    trenuri[i].data_sosire, trenuri[i].ora_sosire, trenuri[i].intarziere_plecare, trenuri[i].intarziere_sosire, trenuri[i].mai_devreme);
            strcat(mesaj, msg);
            if ((atoi(trenuri[i].intarziere_plecare) + atoi(trenuri[i].intarziere_sosire) - atoi(trenuri[i].mai_devreme)) == 0)
            {
                strcpy(msg, "\e[1;32mTrenul va sosi la timp\e[0m \n");
                strcat(mesaj, msg);
            }
            else if ((atoi(trenuri[i].intarziere_plecare) + atoi(trenuri[i].intarziere_sosire) - atoi(trenuri[i].mai_devreme)) > 0)
            {
                strcpy(time, trenuri[i].ora_sosire);
                getTimeAdunare(time, (atoi(trenuri[i].intarziere_plecare) + atoi(trenuri[i].intarziere_sosire) - atoi(trenuri[i].mai_devreme)));
                sprintf(msg, "\e[1;31mTrenul va ajunge cu o intarziere de %d minute la %s\e[0m \n", (atoi(trenuri[i].intarziere_plecare) + atoi(trenuri[i].intarziere_sosire) - atoi(trenuri[i].mai_devreme)), time);
                strcat(mesaj, msg);
            }
            else
            {
                strcpy(time, trenuri[i].ora_sosire);
                printf("timp = %d\n",atoi(trenuri[i].intarziere_sosire) + atoi(trenuri[i].intarziere_plecare) - atoi(trenuri[i].mai_devreme));
                getTimeAdunare(time, atoi(trenuri[i].intarziere_sosire) + atoi(trenuri[i].intarziere_plecare) - atoi(trenuri[i].mai_devreme));
                sprintf(msg, "\e[1;31mTrenul va ajunge mai devreme cu %d minute la %s\e[0m \n", atoi(trenuri[i].mai_devreme) - (atoi(trenuri[i].intarziere_sosire) + atoi(trenuri[i].intarziere_plecare)), time);
                strcat(mesaj, msg);
            }
            strcat(mesaj, "\e[1;32m=====================================================================\e[0m \n");
            break;
        }
    if (g == 0)
    {
        sprintf(msg, "\e[1;31m[server]Nu este niciun tren cu id-ul %s.\e[0m", id);
        strcpy(mesaj, msg);
    }
}
//===================================================================================================================//
void raspunde(void *arg)
{
    int nr, i = 0;
    struct thData tdL;
    tdL = *((struct thData *)arg);
    char mesaj[8192];
    char msgrasp[8192];
    int indicator_logare = 0;
    char username[256];
    while (1)
    {

        bzero(mesaj, 8192);
        printf("Asteptam mesajul...\n");
        fflush(stdout);

        if (read(tdL.cl, mesaj, 8192) <= 0)
        {
            printf("[Thread %d]\n", tdL.idThread);
            perror("Eroare la read() de la client.\n");
        }

        printf("Mesajul a fost receptionat...%s\n", mesaj);
        construire_comanda(mesaj);
        bzero(msgrasp, 8192);
        if (strcmp(comanda[1], "quit") == 0 || strcmp(comanda[1], "13.") == 0) // quit
        {
            if (indicator_logare == 1)
            {
                schimbaStatus(username, "0");
            }
            break;
        }
        else if (strcmp(comanda[1], "comenzi") == 0) // comenzi
            strcpy(msgrasp, "[server]Comenzi disponibile:\n\e[1;32ma.Comenzi pentru useri nelogati\e[0m\n1.login <username> <parola>\n2.sign-in <user> <parola>\n3.quit\n\e[1;32mb.Comenzi pentru useri logati (acestea pot fi apelate si cu <id.>): \e[0m\n1.getInfoAstazi - Trenurile care pleaca astazi\n2.getPlecari - Trenurile care pleaca in ora urmatoare\n3.getSosiri - Trenurile care sosesc in ora urmatoare\n4.setIntarzierePlecare <id_tren> <minute>\n5.setIntarziereSosire <id_tren> <minute>\n6.setSosireDevreme <id_tren> <minute>\n7.getOrasPlecare <oras_plecare>\n8.getOrasSosire <oras_sosire>\n9.getStatie 'plecare' | 'sosire'\n10.ruteTrenuri <statie_plecare> | <statie_sosire>\n11.getTrenById <id_tren>\n12.logout\n13.quit\n");
        else if (strcmp(comanda[1], "login") == 0 && (nr_parametri > 4 || nr_parametri < 4)) // login
            strcpy(msgrasp, "\e[1;31m[server]Utilizare gresita a comenzii! Formatul este login <username> <parola>\e[0m");
        else if (strcmp(comanda[1], "login") == 0 && (nr_parametri == 4) && indicator_logare == 1)
        {
            strcpy(msgrasp, "\e[1;31m[server]Sunteti deja logat cu username-ul: ");
            strcat(msgrasp, username);
            strcat(msgrasp, " \e[0m");
        }
        else if (strcmp(comanda[1], "login") == 0 && (nr_parametri == 4) && indicator_logare == 0)
        {
            logare(msgrasp, &indicator_logare, username);
        }
        else if (strcmp(comanda[1], "sign-in") == 0 && (nr_parametri == 4) && indicator_logare == 1) // sign-in
        {
            strcpy(msgrasp, "[server]Pentru a va putea inregistra trebuie ma intai sa va delogati");
        }
        else if (strcmp(comanda[1], "sign-in") == 0 && (nr_parametri > 4 || nr_parametri < 4))
            strcpy(msgrasp, "\e[1;31m[server]Utilizare gresita a comenzii! Formatul este sign-in <username> <parola>\e[0m");
        else if (strcmp(comanda[1], "sign-in") == 0 && (nr_parametri == 4) && indicator_logare == 0)
        {
            sign_in(msgrasp, &indicator_logare, username);
        }
        else if ((strcmp(comanda[1], "logout") == 0 || strcmp(comanda[1], "12.") == 0) && (nr_parametri == 2) && indicator_logare == 1) // logout
        {
            logout(msgrasp, &indicator_logare, username);
            strcpy(msgrasp, "\e[1;32m[server]V-ati delogat cu succes!\e[0m");
        }
        else if ((strcmp(comanda[1], "logout") == 0 || strcmp(comanda[1], "12.") == 0) && (nr_parametri == 2) && indicator_logare == 0)
        {
            strcpy(msgrasp, "\e[1;31m[server]Nu sunteti logat cu niciun username!\e[0m");
        }
        else if ((strcmp(comanda[1], "logout") == 0 || strcmp(comanda[1], "12.") == 0) && (nr_parametri > 2))
        {
            strcpy(msgrasp, "\e[1;31m[server]Utilizare gresita a comenzii! Formatul este: logout\e[0m");
        }
        else if ((strcmp(comanda[1], "getInfoAstazi") == 0 || strcmp(comanda[1], "1.") == 0) && (nr_parametri == 2) && indicator_logare == 1)
        {
            getInfoAstazi(msgrasp);
        }
        else if ((strcmp(comanda[1], "getInfoAstazi") == 0 || strcmp(comanda[1], "1.") == 0) && (nr_parametri == 2) && indicator_logare == 0)
        {
            strcpy(msgrasp, "\e[1;31m[server]Pentru a putea utiliza aceasta comanda trebuie sa fiti logat intr-un cont!\e[0m");
        }
        else if ((strcmp(comanda[1], "getPlecari") == 0 || strcmp(comanda[1], "2.") == 0) && (nr_parametri == 2) && indicator_logare == 1)
        {
            getPlecari(msgrasp);
        }
        else if ((strcmp(comanda[1], "getPlecari") == 0 || strcmp(comanda[1], "2.") == 0) && (nr_parametri == 2) && indicator_logare == 0)
        {
            strcpy(msgrasp, "\e[1;31m[server]Pentru a putea utiliza aceasta comanda trebuie sa fiti logat intr-un cont!\e[0m");
        }
        else if ((strcmp(comanda[1], "getSosiri") == 0 || strcmp(comanda[1], "3.") == 0) && (nr_parametri == 2) && indicator_logare == 1)
        {
            getSosiri(msgrasp);
        }
        else if ((strcmp(comanda[1], "getSosiri") == 0 || strcmp(comanda[1], "3.") == 0) && indicator_logare == 0)
        {
            strcpy(msgrasp, "\e[1;31m[server]Pentru a putea utiliza aceasta comanda trebuie sa fiti logat intr-un cont!\e[0m");
        }
        else if ((strcmp(comanda[1], "setIntarzierePlecare") == 0 || strcmp(comanda[1], "4.") == 0) && (nr_parametri == 4) && indicator_logare == 1)
        {
            setIntarzierePlecare(msgrasp);
        }
        else if ((strcmp(comanda[1], "setIntarzierePlecare") == 0 || strcmp(comanda[1], "4.") == 0) && (nr_parametri > 4 || nr_parametri < 4) && (indicator_logare == 1 || indicator_logare == 0))
        {
            strcpy(msgrasp, "\e[1;31m[server]Utilizare gresita a comenzii! Formatul este setIntarzierePlecare <id_tren> <Intarziere_Plecare in minute>\e[0m");
        }
        else if ((strcmp(comanda[1], "setIntarzierePlecare") == 0 || strcmp(comanda[1], "4.") == 0) && (nr_parametri == 4) && indicator_logare == 0)
        {
            strcpy(msgrasp, "\e[1;31m[server]Pentru a putea utiliza aceasta comanda trebuie sa fiti logat intr-un cont!\e[0m");
        }
        else if ((strcmp(comanda[1], "setIntarziereSosire") == 0 || strcmp(comanda[1], "5.") == 0) && (nr_parametri == 4) && indicator_logare == 1)
        {
            setIntarziereSosire(msgrasp);
        }
        else if ((strcmp(comanda[1], "setIntarziereSosire") == 0 || strcmp(comanda[1], "5.") == 0) && (nr_parametri > 4 || nr_parametri < 4) && (indicator_logare == 1 || indicator_logare == 0))
        {
            strcpy(msgrasp, "\e[1;31m[server]Utilizare gresita a comenzii! Formatul este setIntarziereSosire <id_tren> <Intarziere_Plecare in minute>\e[0m");
        }
        else if ((strcmp(comanda[1], "setIntarziereSosire") == 0 || strcmp(comanda[1], "5.") == 0) && (nr_parametri == 4) && indicator_logare == 0)
        {
            strcpy(msgrasp, "\e[1;31m[server]Pentru a putea utiliza aceasta comanda trebuie sa fiti logat intr-un cont!\e[0m");
        }
        else if ((strcmp(comanda[1], "setSosireDevreme") == 0 || strcmp(comanda[1], "6.") == 0) && (nr_parametri == 4) && indicator_logare == 1)
        {
            setSosireDevreme(msgrasp);
        }
        else if ((strcmp(comanda[1], "setSosireDevreme") == 0 || strcmp(comanda[1], "6.") == 0) && (nr_parametri > 4 || nr_parametri < 4) && (indicator_logare == 1 || indicator_logare == 0))
        {
            strcpy(msgrasp, "\e[1;31m[server]Utilizare gresita a comenzii! Formatul este setSosireDevreme <id_tren> <Mai devreme in minute>\e[0m");
        }
        else if ((strcmp(comanda[1], "setSosireDevreme") == 0 || strcmp(comanda[1], "6.") == 0) && (nr_parametri == 4) && indicator_logare == 0)
        {
            strcpy(msgrasp, "\e[1;31m[server]Pentru a putea utiliza aceasta comanda trebuie sa fiti logat intr-un cont!\e[0m");
        }
        else if ((strcmp(comanda[1], "getOrasPlecare") == 0 || strcmp(comanda[1], "7.") == 0) && (nr_parametri == 3) && indicator_logare == 1)
        {
            getOrasPlecare(msgrasp);
        }
        else if ((strcmp(comanda[1], "getOrasPlecare") == 0 || strcmp(comanda[1], "7.") == 0) && (nr_parametri > 3 || nr_parametri < 3) && (indicator_logare == 1 || indicator_logare == 0))
        {
            strcpy(msgrasp, "\e[1;31m[server]Utilizare gresita a comenzii! Formatul este getOrasPlecare <oras_plecare>\e[0m");
        }
        else if ((strcmp(comanda[1], "getOrasPlecare") == 0 || strcmp(comanda[1], "7.") == 0) && (nr_parametri == 3) && indicator_logare == 0)
        {
            strcpy(msgrasp, "\e[1;31m[server]Pentru a putea utiliza aceasta comanda trebuie sa fiti logat intr-un cont!\e[0m");
        }
        else if ((strcmp(comanda[1], "getOrasSosire") == 0 || strcmp(comanda[1], "8.") == 0) && (nr_parametri == 3) && indicator_logare == 1)
        {
            getOrasSosire(msgrasp);
        }
        else if ((strcmp(comanda[1], "getOrasSosire") == 0 || strcmp(comanda[1], "8.") == 0) && (nr_parametri > 3 || nr_parametri < 3) && (indicator_logare == 1 || indicator_logare == 0))
        {
            strcpy(msgrasp, "\e[1;31m[server]Utilizare gresita a comenzii! Formatul este getOrasSosire <oras_plecare>\e[0m");
        }
        else if ((strcmp(comanda[1], "getOrasSosire") == 0 || strcmp(comanda[1], "8.") == 0) && (nr_parametri == 3) && indicator_logare == 0)
        {
            strcpy(msgrasp, "\e[1;31m[server]Pentru a putea utiliza aceasta comanda trebuie sa fiti logat intr-un cont!\e[0m");
        }
        else if ((strcmp(comanda[1], "getStatie") == 0 || strcmp(comanda[1], "9.") == 0) && (nr_parametri == 3) && indicator_logare == 1)
        {
            getStatii(msgrasp);
        }
        else if ((strcmp(comanda[1], "getStatie") == 0 || strcmp(comanda[1], "9.") == 0) && (nr_parametri > 3 || nr_parametri < 3) && (indicator_logare == 1 || indicator_logare == 0))
        {
            strcpy(msgrasp, "\e[1;31m[server]Utilizare gresita a comenzii! Formatul este getStatie plecare | sosire\e[0m");
        }
        else if ((strcmp(comanda[1], "getStatie") == 0 || strcmp(comanda[1], "9.") == 0) && (nr_parametri == 3) && indicator_logare == 0)
        {
            strcpy(msgrasp, "\e[1;31m[server]Pentru a putea utiliza aceasta comanda trebuie sa fiti logat intr-un cont!\e[0m");
        }
        else if ((strcmp(comanda[1], "ruteTrenuri") == 0 || strcmp(comanda[1], "10.") == 0) && (nr_parametri == 4) && indicator_logare == 1)
        {
            ruteTrenuri(msgrasp);
        }
        else if ((strcmp(comanda[1], "ruteTrenuri") == 0 || strcmp(comanda[1], "10.") == 0) && (nr_parametri > 4 || nr_parametri < 4) && (indicator_logare == 1 || indicator_logare == 0))
        {
            strcpy(msgrasp, "\e[1;31m[server]Utilizare gresita a comenzii! Formatul este ruteTrenuri <statie_plecare> | <statie_sosire>");
        }
        else if ((strcmp(comanda[1], "ruteTrenuri") == 0 || strcmp(comanda[1], "10.") == 0) && (nr_parametri == 4) && indicator_logare == 0)
        {
            strcpy(msgrasp, "\e[1;31m[server]Pentru a putea utiliza aceasta comanda trebuie sa fiti logat intr-un cont!\e[0m");
        }
        else if ((strcmp(comanda[1], "getTrenById") == 0 || strcmp(comanda[1], "11.") == 0) && (nr_parametri == 3) && indicator_logare == 1)
        {
            getTrenById(msgrasp);
        }
        else if ((strcmp(comanda[1], "getTrenById") == 0 || strcmp(comanda[1], "11.") == 0) && (nr_parametri > 3 || nr_parametri < 3) && (indicator_logare == 1 || indicator_logare == 0))
        {
            strcpy(msgrasp, "\e[1;31m[server]Utilizare gresita a comenzii! Formatul este getTrenById <id_tren>");
        }
        else if ((strcmp(comanda[1], "getTrenById") == 0 || strcmp(comanda[1], "11.") == 0) && (nr_parametri == 3) && indicator_logare == 0)
        {
            strcpy(msgrasp, "\e[1;31m[server]Pentru a putea utiliza aceasta comanda trebuie sa fiti logat intr-un cont!\e[0m");
        }
        else
        {
            strcpy(msgrasp, "[server]Comanda gresita\n");
        }

        printf("Trimitem mesajul inapoi...\n%s\n", msgrasp);

        if (write(tdL.cl, msgrasp, 8192) <= 0)
        {
            perror("Eroare la write() catre client.\n");
            continue;
        }
        else
            printf("Mesajul a fost trasmis cu succes.\n");
    }
}
static void *treat(void *arg)
{
    struct thData tdL;
    tdL = *((struct thData *)arg);
    printf("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
    fflush(stdout);
    pthread_detach(pthread_self());
    raspunde((struct thData *)arg);
    /* am terminat cu acest client, inchidem conexiunea */
    close((intptr_t)arg);
    return (NULL);
};
int main()
{
    char msgrasp[8192] = " ";
    char msg[256];
    struct sockaddr_in server;
    struct sockaddr_in from;
    setDataBase();
    parse_xml("trenuriAzi.xml");
    int nr;            // mesajul primit de trimis la client
    int sd;            // descriptorul de socket
    pthread_t th[100]; // Identificatorii thread-urilor care se vor crea
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
    if (listen(sd, 5) == -1)
    {
        perror("[server]Eroare la listen().\n");
        return errno;
    }
    /* servim in mod concurent clientii...folosind thread-uri */
    while (1)
    {
        int client;
        thData *td; // parametru functia executata de thread
        int length = sizeof(from);

        printf("[server]Asteptam la portul %d...\n", PORT);
        fflush(stdout);

        /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
        if ((client = accept(sd, (struct sockaddr *)&from, &length)) < 0)
        {
            perror("[server]Eroare la accept().\n");
            continue;
        }
        /* s-a realizat conexiunea, se astepta mesajul */

        // int idThread; //id-ul threadului
        // int cl; //descriptorul intors de accept

        td = (struct thData *)malloc(sizeof(struct thData));
        td->idThread = i++;
        td->cl = client;

        pthread_create(&th[i], NULL, &treat, td);

    } // while
}
