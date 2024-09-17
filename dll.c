//Copyright Caruntu Dana-Maria 311CA
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "dll.h"
#include "command.h"
#include "struct.h"

/*
* Functie care trebuie apelata pentru alocarea si initializarea unei liste.
* (Setare valori initiale pentru campurile specifice structurii list).
*/
dll_list*
dll_create(unsigned int data_size, int bytes_per_list)
{
	dll_list *list = (dll_list *)malloc(sizeof(dll_list));
	DIE(!list, "malloc failed");
	list->head = NULL;
	list->size = bytes_per_list / data_size;
	list->data_size = data_size;
	return list;
}

/*
* Functia intoarce un pointer la nodul de pe pozitia n din lista.
* Pozitiile din lista sunt indexate incepand cu 0 (i.e. primul nod din lista se
* afla pe pozitia n=0). Daca n >= nr_noduri, atunci se intoarce nodul de pe
* pozitia rezultata daca am "cicla" (posibil de mai multe ori) pe lista si am
* trece de la ultimul nod, inapoi la primul si am continua de acolo.
*/
dll_node_t *dll_get_nth_node(dll_list *list, unsigned int n)
{
	if (!list->head || !list)
		return NULL; //caz in care lista mea este goala
	dll_node_t *current = list->head;
	for (unsigned int i = 0; i < n; i++) {
		if (!current->next) {//deoarece lista nu este circulara
			current = list->head;//pun conditie pentru "structura toroidala"
			//adica daca ajunge la final (NULL) este chiar head-ul listei,
			//si se intoarce de la inceput
		} else {
			current = current->next;
		}
	}
	return current;
}

/*
* Pe baza datelor trimise prin pointerul new_data, se creeaza un nou nod care e
* adaugat pe pozitia n a listei reprezentata de pointerul list. Pozitiile din
* lista sunt indexate incepand cu 0 (i.e. primul nod din lista se afla pe
* pozitia n=0). Cand indexam pozitiile nu "ciclam" pe lista circulara ca la
* get, ci consideram nodurile in ordinea de la head la ultimul (adica acel nod
* care pointeaza la head ca nod urmator in lista). Daca n >= nr_noduri, atunci
* adaugam nodul nou la finalul listei.
*/

void dll_add_nth_node(dll_list *list, int n, const void *data)
{
	dll_node_t *node = list->head;
	dll_node_t *new_node = malloc(sizeof(dll_node_t));
	DIE(!new_node, "malloc failed");
	new_node->data = malloc(sizeof(unsigned long));
	DIE(!new_node->data, "malloc failed");
	memcpy(new_node->data, data, sizeof(unsigned long));
	new_node->next = NULL;
	new_node->prev = NULL;
	if (n == 0) {
		new_node->next = list->head;
		if (list->head)
			list->head->prev = new_node;
		list->head = new_node;
		return;
	}
	if (!node) {
		list->head = new_node;
		return;
	}
	for (int i = 0; node && i < n - 1; ++i) {
		if (!node->next) {
			node->next = new_node;
			new_node->prev = node;
			return;
		}
		node = node->next;
	}
	if (!node->next) {
		node->next = new_node;
		new_node->prev = node;
		return;
	}
	new_node->prev = node;
	new_node->next = node->next;
	node->next = new_node;
	new_node->next->prev = new_node;
}

void add_nth_block(allocated_block_info *blocks, unsigned int n,
				   unsigned long address, int n_bytes)
{
	blocks->data_size += n_bytes;
	block_info *new_block = malloc(sizeof(block_info));
	DIE(!new_block, "malloc failed");
	new_block->address = address;
	new_block->n_bytes = n_bytes;
	new_block->input = NULL;
	new_block->next = NULL;
	new_block->prev = NULL;
	block_info *start = blocks->head;
	if (!start || n == 0) {
		blocks->head = new_block;
		new_block->next = start;
		if (start)
			start->prev = new_block;
		blocks->size++;
		return;
	}
	if (n > blocks->size)
		n = blocks->size;
	for (unsigned int i = 0; i < n - 1; ++i)
		start = start->next;
	new_block->prev = start;
	new_block->next = start->next;
	start->next = new_block;
	if (new_block->next)
		new_block->next->prev = new_block;
	new_block->address = address;
	new_block->n_bytes = n_bytes;
	blocks->size++;
}

dll_node_t *dll_remove_nth_node(dll_list *list, unsigned int n)
{
	if (n == 0) {
		dll_node_t *ans = list->head;
		list->head = list->head->next;
		return ans;
	}
	dll_node_t *node = list->head;
	for (unsigned int i = 0; i < n - 2; ++i) {
		if (!node->next) {
			node->prev->next = NULL;
			return node;
		}
		node = node->next;
	}
	if (!node->next) {
		node->prev->next = NULL;
		return node;
	}
	node->prev->next = node->next;
	node->next->prev = node->prev;
	return node;
}

/*
* Functia intoarce numarul de noduri din lista al carei pointer este trimis ca
* parametru.
*/
unsigned int dll_get_size(dll_list *list)
{
	dll_node_t *curr = list->head;
	unsigned int count = 0;
	while (curr) {
		count++;
		curr = curr->next;
	}
	return count;
}

/*
* Procedura elibereaza memoria folosita de toate nodurile din lista, iar la
* sfarsit, elibereaza memoria folosita de structura lista.
*/
void dll_free(dll_list **pp_list)
{
	if (!pp_list || !(*pp_list))
		return; //verific daca lista este goala
	dll_list *list = *pp_list;
	//mai sus deferentiez pointerul pentru a obtine pointerul
	//de la lista inlantuita pe care vr sa o eliberez
	//pp_list este un pointer la un pointer catre o structra
	dll_node_t *current = list->head;
	//pornind de la head, parcurg si eliberez
	//memoria folosita de fiecare nod al listei
	while (current) {
		dll_node_t *next = current->next;
		free(current->data);
		free(current);
		current = next;
	}
	//eliberarea resurselor si stergerea listei
	free(list);
	*pp_list = NULL;
}

void dll_free_block(allocated_block_info **block)
{
	if (!block || !(*block))
		return; //verific daca lista este goala
	allocated_block_info *list = *block;
	//mai sus deferentiez pointerul pt a obtine pointerul
	//de la lista inlantuita pe care vr sa o eliberez
	//pp_list este un pointer la un pointer catre o structra
	block_info *current = list->head;
	//pornind de la head, parcurg si eliberez
	//memoria folosita de fiecare nod al listei
	while (current) {
		block_info *next = current->next;
		if (current->input)
			free(current->input);
		free(current);
		current = next;
	}
	//eliberarea resurselor si stergerea listei
	free(list);
	*block = NULL;
}

