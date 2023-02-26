#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

// Structura pentru a salva trenuri[i]rmatiile despre fiecare tren
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
} tren;
tren trenuri[100];
int numar_trenuri;
// Functie pentru a parsa fisierul XML
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
void modifica_struct(char *numar, char *field, char *value)
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
      break;
    }
}
// Functie pentru a modifica fisierul XML
void modify_xml(char *filename, char *numar, char *field, char *value)
{
  // Incarcam fisierul XML in memorie
  xmlDoc *doc = xmlReadFile(filename, NULL, 0);
  if (doc == NULL)
  {
    printf("Eroare la incarcarea fisierului XML '%s'\n", filename);
    return;
  }

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
      else
      {
        printf("Tren inexistent\n");
        exit(1);
      }
    }
  }

  // Salvam modificarile in fisierul XML
  xmlSaveFormatFileEnc(filename, doc, "UTF-8", 1);

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
int main(int argc, char **argv)
{
  char oras[256];
  char ora[256];
  // Apelam functia parse_xml cu numele fisierului XML ca argument
  parse_xml("6.xml");
  for (int i = 0; i < numar_trenuri; i++)
    printf("Trenul %s: plecare din %s spre %s la %s %s, sosire la %s %s (intarziere plecare: %s minute, intarziere sosire: %s minute)\n",
           trenuri[i].numar, trenuri[i].oras_plecare, trenuri[i].oras_sosire, trenuri[i].data_plecare, trenuri[i].ora_plecare,
           trenuri[i].data_sosire, trenuri[i].ora_sosire, trenuri[i].intarziere_plecare, trenuri[i].intarziere_sosire);
  modify_xml("6.xml", "2", "ora_plecare", "17:30");
  extract_field("6.xml", "2", "oras_plecare", oras);
  extract_field("6.xml", "2", "ora_sosire", ora);
  printf("Ora de plecare pentru trenul care pleaca din %s este %s\n", oras, ora);
  return 0;
}
