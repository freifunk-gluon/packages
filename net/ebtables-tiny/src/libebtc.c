/*
 * libebtc.c, January 2004
 *
 * Contains the functions with which to make a table in userspace.
 *
 * Author: Bart De Schuymer
 *
 *  This code is stongly inspired on the iptables code which is
 *  Copyright (C) 1999 Paul `Rusty' Russell & Michael J. Neuling
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "include/ebtables_u.h"
#include "include/ethernetdb.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>

static void decrease_chain_jumps(struct ebt_u_replace *replace);
static int iterate_entries(struct ebt_u_replace *replace, int type);

/* The standard names */
const char *ebt_hooknames[NF_BR_NUMHOOKS] =
{
	[NF_BR_PRE_ROUTING]"PREROUTING",
	[NF_BR_LOCAL_IN]"INPUT",
	[NF_BR_FORWARD]"FORWARD",
	[NF_BR_LOCAL_OUT]"OUTPUT",
	[NF_BR_POST_ROUTING]"POSTROUTING",
	[NF_BR_BROUTING]"BROUTING"
};

/* The four target names */
const char* ebt_standard_targets[NUM_STANDARD_TARGETS] =
{
	"ACCEPT",
	"DROP",
	"CONTINUE",
	"RETURN",
};

/* The lists of supported tables, matches, watchers and targets */
struct ebt_u_table *ebt_tables;
struct ebt_u_match *ebt_matches;
struct ebt_u_target *ebt_targets;

/* Find the right structure belonging to a name */
struct ebt_u_target *ebt_find_target(const char *name)
{
	struct ebt_u_target *t = ebt_targets;

	while (t && strcmp(t->name, name))
		t = t->next;
	return t;
}

struct ebt_u_match *ebt_find_match(const char *name)
{
	struct ebt_u_match *m = ebt_matches;

	while (m && strcmp(m->name, name))
		m = m->next;
	return m;
}

struct ebt_u_table *ebt_find_table(const char *name)
{
	struct ebt_u_table *t = ebt_tables;

	while (t && strcmp(t->name, name))
		t = t->next;
	return t;
}

/* Prints all registered extensions */
void ebt_list_extensions()
{
	struct ebt_u_table *tbl = ebt_tables;
        struct ebt_u_target *t = ebt_targets;
        struct ebt_u_match *m = ebt_matches;

	PRINT_VERSION;
	printf("Loaded userspace extensions:\n\nLoaded tables:\n");
        while (tbl) {
		printf("%s\n", tbl->name);
                tbl = tbl->next;
	}
	printf("\nLoaded targets:\n");
        while (t) {
		printf("%s\n", t->name);
                t = t->next;
	}
	printf("\nLoaded matches:\n");
        while (m) {
		printf("%s\n", m->name);
                m = m->next;
	}
}

#ifndef LOCKFILE
#define LOCKDIR "/var/lib/ebtables"
#define LOCKFILE LOCKDIR"/lock"
#endif
/* Returns 0 on success, -1 when the file is locked by another process
 * or -2 on any other error. */
static int lock_file()
{
	int fd, try = 0;
	int ret = 0;

retry:
	fd = open(LOCKFILE, O_CREAT, 00600);
	if (fd < 0) {
		if (try == 1 || mkdir(LOCKDIR, 00700))
			return -2;
		try = 1;
		goto retry;
	}
	ret = flock(fd, LOCK_EX | LOCK_NB);
	if (ret)
		close(fd);
	return ret;
}

/* Get the table from the kernel or from a binary file */
int ebt_get_kernel_table(struct ebt_u_replace *replace)
{
	int ret;
	struct timespec req = {0};
	int errcount = 0;

	if (!ebt_find_table(replace->name)) {
		ebt_print_error("Bad table name '%s'", replace->name);
		return -1;
	}
	while ((ret = lock_file())) {
		if (ret == -2) {
			/* if we get an error we can't handle, we exit. This
			 * doesn't break backwards compatibility since using
			 * this file locking is disabled by default. */
			ebt_print_error2("Unable to create lock file "LOCKFILE);
		}
		if (errcount++ % 20 == 0) {
			fprintf(stderr, "Trying to obtain lock %s\n", LOCKFILE);
		}
		req.tv_sec = 0;
		req.tv_nsec = 50*1000*1000; //50ms
		nanosleep(&req, NULL);
	}
	/* Get the kernel's information */
	if (ebt_get_table(replace))
		ebt_print_error("The kernel doesn't support the ebtables '%s' table", replace->name);
	return 0;
}

/* Put sane values into a new entry */
void ebt_initialize_entry(struct ebt_u_entry *e)
{
	e->bitmask = EBT_NOPROTO;
	e->invflags = 0;
	e->ethproto = 0;
	strcpy(e->in, "");
	strcpy(e->out, "");
	strcpy(e->logical_in, "");
	strcpy(e->logical_out, "");
	e->m_list = NULL;
	e->t = (struct ebt_entry_target *)ebt_find_target(EBT_STANDARD_TARGET);
	ebt_find_target(EBT_STANDARD_TARGET)->used = 1;
	e->cnt.pcnt = e->cnt.bcnt = e->cnt_surplus.pcnt = e->cnt_surplus.bcnt = 0;

	if (!e->t)
		ebt_print_bug("Couldn't load standard target");
	((struct ebt_standard_target *)((struct ebt_u_target *)e->t)->t)->verdict = EBT_CONTINUE;
}


/* This doesn't free e, because the calling function might need e->next */
void ebt_free_u_entry(struct ebt_u_entry *e)
{
	struct ebt_u_match_list *m_l, *m_l2;

	m_l = e->m_list;
	while (m_l) {
		m_l2 = m_l->next;
		free(m_l->m);
		free(m_l);
		m_l = m_l2;
	}
	free(e->t);
}

/* Parse the chain name and return the corresponding chain nr
 * returns -1 on failure */
int ebt_get_chainnr(const struct ebt_u_replace *replace, const char* arg)
{
	int i;

	for (i = 0; i < replace->num_chains; i++) {
		if (!replace->chains[i])
			continue;
		if (!strcmp(arg, replace->chains[i]->name))
			return i;
	}
	return -1;
}

     /*
************
************
**COMMANDS**
************
************
     */

/* Change the policy of selected_chain.
 * Handing a bad policy to this function is a bug. */
void ebt_change_policy(struct ebt_u_replace *replace, int policy)
{
	struct ebt_u_entries *entries = ebt_to_chain(replace);

	if (policy < -NUM_STANDARD_TARGETS || policy == EBT_CONTINUE)
		ebt_print_bug("Wrong policy: %d", policy);
	entries->policy = policy;
}

void ebt_delete_cc(struct ebt_cntchanges *cc)
{
	if (cc->type == CNT_ADD) {
		cc->prev->next = cc->next;
		cc->next->prev = cc->prev;
		free(cc);
	} else
		cc->type = CNT_DEL;
}

void ebt_empty_chain(struct ebt_u_entries *entries)
{
	struct ebt_u_entry *u_e = entries->entries->next, *tmp;
	while (u_e != entries->entries) {
		ebt_delete_cc(u_e->cc);
		ebt_free_u_entry(u_e);
		tmp = u_e->next;
		free(u_e);
		u_e = tmp;
	}
	entries->entries->next = entries->entries->prev = entries->entries;
	entries->nentries = 0;
}

/* Flush one chain or the complete table
 * If selected_chain == -1 then flush the complete table */
void ebt_flush_chains(struct ebt_u_replace *replace)
{
	int i, numdel;
	struct ebt_u_entries *entries = ebt_to_chain(replace);

	/* Flush whole table */
	if (!entries) {
		if (replace->nentries == 0)
			return;
		replace->nentries = 0;

		/* Free everything and zero (n)entries */
		for (i = 0; i < replace->num_chains; i++) {
			if (!(entries = replace->chains[i]))
				continue;
			entries->counter_offset = 0;
			ebt_empty_chain(entries);
		}
		return;
	}

	if (entries->nentries == 0)
		return;
	replace->nentries -= entries->nentries;
	numdel = entries->nentries;

	/* Update counter_offset */
	for (i = replace->selected_chain+1; i < replace->num_chains; i++) {
		if (!(entries = replace->chains[i]))
			continue;
		entries->counter_offset -= numdel;
	}

	entries = ebt_to_chain(replace);
	ebt_empty_chain(entries);
}

/* Returns the rule number on success (starting from 0), -1 on failure
 *
 * This function expects the ebt_{match,watcher,target} members of new_entry
 * to contain pointers to ebt_u_{match,watcher,target} */
int ebt_check_rule_exists(struct ebt_u_replace *replace,
			  struct ebt_u_entry *new_entry)
{
	struct ebt_u_entry *u_e;
	struct ebt_u_match_list *m_l, *m_l2;
	struct ebt_u_match *m;
	struct ebt_u_target *t = (struct ebt_u_target *)new_entry->t;
	struct ebt_u_entries *entries = ebt_to_chain(replace);
	int i, j, k;

	u_e = entries->entries->next;
	/* Check for an existing rule (if there are duplicate rules,
	 * take the first occurance) */
	for (i = 0; i < entries->nentries; i++, u_e = u_e->next) {
		if (u_e->ethproto != new_entry->ethproto)
			continue;
		if (strcmp(u_e->in, new_entry->in))
			continue;
		if (strcmp(u_e->out, new_entry->out))
			continue;
		if (strcmp(u_e->logical_in, new_entry->logical_in))
			continue;
		if (strcmp(u_e->logical_out, new_entry->logical_out))
			continue;
		if (new_entry->bitmask & EBT_SOURCEMAC &&
		    memcmp(u_e->sourcemac, new_entry->sourcemac, ETH_ALEN))
			continue;
		if (new_entry->bitmask & EBT_DESTMAC &&
		    memcmp(u_e->destmac, new_entry->destmac, ETH_ALEN))
			continue;
		if (new_entry->bitmask != u_e->bitmask ||
		    new_entry->invflags != u_e->invflags)
			continue;
		/* Compare all matches */
		m_l = new_entry->m_list;
		j = 0;
		while (m_l) {
			m = (struct ebt_u_match *)(m_l->m);
			m_l2 = u_e->m_list;
			while (m_l2 && strcmp(m_l2->m->u.name, m->m->u.name))
				m_l2 = m_l2->next;
			if (!m_l2 || !m->compare(m->m, m_l2->m))
				goto letscontinue;
			j++;
			m_l = m_l->next;
		}
		/* Now be sure they have the same nr of matches */
		k = 0;
		m_l = u_e->m_list;
		while (m_l) {
			k++;
			m_l = m_l->next;
		}
		if (j != k)
			continue;

		if (strcmp(t->t->u.name, u_e->t->u.name))
			continue;
		if (!t->compare(t->t, u_e->t))
			continue;
		return i;
letscontinue:;
	}
	return -1;
}

/* Add a rule, rule_nr is the rule to update
 * rule_nr specifies where the rule should be inserted
 * rule_nr > 0 : insert the rule right before the rule_nr'th rule
 *               (the first rule is rule 1)
 * rule_nr < 0 : insert the rule right before the (n+rule_nr+1)'th rule,
 *               where n denotes the number of rules in the chain
 * rule_nr == 0: add a new rule at the end of the chain
 *
 * This function expects the ebt_{match,watcher,target} members of new_entry
 * to contain pointers to ebt_u_{match,watcher,target} and updates these
 * pointers so that they point to ebt_{match,watcher,target}, before adding
 * the rule to the chain. Don't free() the ebt_{match,watcher,target} and
 * don't reuse the new_entry after a successful call to ebt_add_rule() */
void ebt_add_rule(struct ebt_u_replace *replace, struct ebt_u_entry *new_entry, int rule_nr)
{
	int i;
	struct ebt_u_entry *u_e;
	struct ebt_u_match_list *m_l;
	struct ebt_u_entries *entries = ebt_to_chain(replace);
	struct ebt_cntchanges *cc, *new_cc;

	if (rule_nr <= 0)
		rule_nr += entries->nentries;
	else
		rule_nr--;
	if (rule_nr > entries->nentries || rule_nr < 0) {
		ebt_print_error("The specified rule number is incorrect");
		return;
	}
	/* Go to the right position in the chain */
	if (rule_nr == entries->nentries)
		u_e = entries->entries;
	else {
		u_e = entries->entries->next;
		for (i = 0; i < rule_nr; i++)
			u_e = u_e->next;
	}
	/* We're adding one rule */
	replace->nentries++;
	entries->nentries++;
	/* Insert the rule */
	new_entry->next = u_e;
	new_entry->prev = u_e->prev;
	u_e->prev->next = new_entry;
	u_e->prev = new_entry;
	new_cc = (struct ebt_cntchanges *)malloc(sizeof(struct ebt_cntchanges));
	if (!new_cc)
		ebt_print_memory();
	new_cc->type = CNT_ADD;
	new_cc->change = 0;
	if (new_entry->next == entries->entries) {
		for (i = replace->selected_chain+1; i < replace->num_chains; i++)
			if (!replace->chains[i] || replace->chains[i]->nentries == 0)
				continue;
			else
				break;
		if (i == replace->num_chains)
			cc = replace->cc;
		else
			cc = replace->chains[i]->entries->next->cc;
	} else
		cc = new_entry->next->cc;
	new_cc->next = cc;
	new_cc->prev = cc->prev;
	cc->prev->next = new_cc;
	cc->prev = new_cc;
	new_entry->cc = new_cc;

	/* Put the ebt_{match, watcher, target} pointers in place */
	m_l = new_entry->m_list;
	while (m_l) {
		m_l->m = ((struct ebt_u_match *)m_l->m)->m;
		m_l = m_l->next;
	}
	new_entry->t = ((struct ebt_u_target *)new_entry->t)->t;
	/* Update the counter_offset of chains behind this one */
	for (i = replace->selected_chain+1; i < replace->num_chains; i++) {
		entries = replace->chains[i];
		if (!(entries = replace->chains[i]))
			continue;
		entries->counter_offset++;
	}
}

/* If *begin==*end==0 then find the rule corresponding to new_entry,
 * else make the rule numbers positive (starting from 0) and check
 * for bad rule numbers. */
static int check_and_change_rule_number(struct ebt_u_replace *replace,
   struct ebt_u_entry *new_entry, int *begin, int *end)
{
	struct ebt_u_entries *entries = ebt_to_chain(replace);

	if (*begin < 0)
		*begin += entries->nentries + 1;
	if (*end < 0)
		*end += entries->nentries + 1;

	if (*begin < 0 || *begin > *end || *end > entries->nentries) {
		ebt_print_error("Sorry, wrong rule numbers");
		return -1;
	}

	if ((*begin * *end == 0) && (*begin + *end != 0))
		ebt_print_bug("begin and end should be either both zero, "
			      "either both non-zero");
	if (*begin != 0) {
		(*begin)--;
		(*end)--;
	} else {
		*begin = ebt_check_rule_exists(replace, new_entry);
		*end = *begin;
		if (*begin == -1) {
			ebt_print_error("Sorry, rule does not exist");
			return -1;
		}
	}
	return 0;
}

/* Delete a rule or rules
 * begin == end == 0: delete the rule corresponding to new_entry
 *
 * The first rule has rule nr 1, the last rule has rule nr -1, etc.
 * This function expects the ebt_{match,watcher,target} members of new_entry
 * to contain pointers to ebt_u_{match,watcher,target}. */
void ebt_delete_rule(struct ebt_u_replace *replace,
		     struct ebt_u_entry *new_entry, int begin, int end)
{
	int i,  nr_deletes;
	struct ebt_u_entry *u_e, *u_e2, *u_e3;
	struct ebt_u_entries *entries = ebt_to_chain(replace);

	if (check_and_change_rule_number(replace, new_entry, &begin, &end))
		return;
	/* We're deleting rules */
	nr_deletes = end - begin + 1;
	replace->nentries -= nr_deletes;
	entries->nentries -= nr_deletes;
	/* Go to the right position in the chain */
	u_e = entries->entries->next;
	for (i = 0; i < begin; i++)
		u_e = u_e->next;
	u_e3 = u_e->prev;
	/* Remove the rules */
	for (i = 0; i < nr_deletes; i++) {
		u_e2 = u_e;
		ebt_delete_cc(u_e2->cc);
		u_e = u_e->next;
		/* Free everything */
		ebt_free_u_entry(u_e2);
		free(u_e2);
	}
	u_e3->next = u_e;
	u_e->prev = u_e3;
	/* Update the counter_offset of chains behind this one */
	for (i = replace->selected_chain+1; i < replace->num_chains; i++) {
		if (!(entries = replace->chains[i]))
			continue;
		entries->counter_offset -= nr_deletes;
	}
}

/* Add a new chain and specify its policy */
void ebt_new_chain(struct ebt_u_replace *replace, const char *name, int policy)
{
	struct ebt_u_entries *new;

	if (replace->num_chains == replace->max_chains)
		ebt_double_chains(replace);
	new = (struct ebt_u_entries *)malloc(sizeof(struct ebt_u_entries));
	if (!new)
		ebt_print_memory();
	replace->chains[replace->num_chains++] = new;
	new->nentries = 0;
	new->policy = policy;
	new->counter_offset = replace->nentries;
	new->hook_mask = 0;
	strcpy(new->name, name);
	new->entries = (struct ebt_u_entry *)malloc(sizeof(struct ebt_u_entry));
	if (!new->entries)
		ebt_print_memory();
	new->entries->next = new->entries->prev = new->entries;
	new->kernel_start = NULL;
}

/* returns -1 if the chain is referenced, 0 on success */
static int ebt_delete_a_chain(struct ebt_u_replace *replace, int chain, int print_err)
{
	int tmp = replace->selected_chain;
	/* If the chain is referenced, don't delete it,
	 * also decrement jumps to a chain behind the
	 * one we're deleting */
	replace->selected_chain = chain;
	if (ebt_check_for_references(replace, print_err))
		return -1;
	decrease_chain_jumps(replace);
	ebt_flush_chains(replace);
	replace->selected_chain = tmp;
	free(replace->chains[chain]->entries);
	free(replace->chains[chain]);
	memmove(replace->chains+chain, replace->chains+chain+1, (replace->num_chains-chain-1)*sizeof(void *));
	replace->num_chains--;
	return 0;
}

/* Selected_chain == -1: delete all non-referenced udc
 * selected_chain < NF_BR_NUMHOOKS is illegal */
void ebt_delete_chain(struct ebt_u_replace *replace)
{
	if (replace->selected_chain != -1 && replace->selected_chain < NF_BR_NUMHOOKS)
		ebt_print_bug("You can't remove a standard chain");
	if (replace->selected_chain == -1) {
		int i = NF_BR_NUMHOOKS;

		while (i < replace->num_chains)
			if (ebt_delete_a_chain(replace, i, 0))
				i++;
	} else
		ebt_delete_a_chain(replace, replace->selected_chain, 1);
}

/* Rename an existing chain. */
void ebt_rename_chain(struct ebt_u_replace *replace, const char *name)
{
	struct ebt_u_entries *entries = ebt_to_chain(replace);

	if (!entries)
		ebt_print_bug("ebt_rename_chain: entries == NULL");
	strcpy(entries->name, name);
}


           /*
*************************
*************************
**SPECIALIZED*FUNCTIONS**
*************************
*************************
            */


void ebt_double_chains(struct ebt_u_replace *replace)
{
	struct ebt_u_entries **new;

	replace->max_chains *= 2;
	new = (struct ebt_u_entries **)malloc(replace->max_chains*sizeof(void *));
	if (!new)
		ebt_print_memory();
	memcpy(new, replace->chains, replace->max_chains/2*sizeof(void *));
	free(replace->chains);
	replace->chains = new;
}

/* Executes the final_check() function for all extensions used by the rule
 * ebt_check_for_loops should have been executed earlier, to make sure the
 * hook_mask is correct. The time argument to final_check() is set to 1,
 * meaning it's the second time the final_check() function is executed. */
void ebt_do_final_checks(struct ebt_u_replace *replace, struct ebt_u_entry *e,
			 struct ebt_u_entries *entries)
{
	struct ebt_u_match_list *m_l;
	struct ebt_u_target *t;
	struct ebt_u_match *m;

	m_l = e->m_list;
	while (m_l) {
		m = ebt_find_match(m_l->m->u.name);
		m->final_check(e, m_l->m, replace->name,
		   entries->hook_mask, 1);
		m_l = m_l->next;
	}
	t = ebt_find_target(e->t->u.name);
	t->final_check(e, e->t, replace->name,
	   entries->hook_mask, 1);
}

/* Returns 1 (if it returns) when the chain is referenced, 0 when it isn't.
 * print_err: 0 (resp. 1) = don't (resp. do) print error when referenced */
int ebt_check_for_references(struct ebt_u_replace *replace, int print_err)
{
	if (print_err)
		return iterate_entries(replace, 1);
	else
		return iterate_entries(replace, 2);
}

struct ebt_u_stack
{
	int chain_nr;
	int n;
	struct ebt_u_entry *e;
	struct ebt_u_entries *entries;
};

/* Checks for loops
 * As a by-product, the hook_mask member of each chain is filled in
 * correctly. The check functions of the extensions need this hook_mask
 * to know from which standard chains they can be called. */
void ebt_check_for_loops(struct ebt_u_replace *replace)
{
	int chain_nr , i, j , k, sp = 0, verdict;
	struct ebt_u_entries *entries, *entries2;
	struct ebt_u_stack *stack = NULL;
	struct ebt_u_entry *e;

	/* Initialize hook_mask to 0 */
	for (i = 0; i < replace->num_chains; i++) {
		if (!(entries = replace->chains[i]))
			continue;
		if (i < NF_BR_NUMHOOKS)
			/* (1 << NF_BR_NUMHOOKS) implies it's a standard chain
			 * (usefull in the final_check() funtions) */
			entries->hook_mask = (1 << i) | (1 << NF_BR_NUMHOOKS);
		else
			entries->hook_mask = 0;
	}
	if (replace->num_chains == NF_BR_NUMHOOKS)
		return;
	stack = (struct ebt_u_stack *)malloc((replace->num_chains - NF_BR_NUMHOOKS) * sizeof(struct ebt_u_stack));
	if (!stack)
		ebt_print_memory();

	/* Check for loops, starting from every base chain */
	for (i = 0; i < NF_BR_NUMHOOKS; i++) {
		if (!(entries = replace->chains[i]))
			continue;
		chain_nr = i;

		e = entries->entries->next;
		for (j = 0; j < entries->nentries; j++) {
			if (strcmp(e->t->u.name, EBT_STANDARD_TARGET))
				goto letscontinue;
			verdict = ((struct ebt_standard_target *)(e->t))->verdict;
			if (verdict < 0)
				goto letscontinue;
			/* Now see if we've been here before */
			for (k = 0; k < sp; k++)
				if (stack[k].chain_nr == verdict + NF_BR_NUMHOOKS) {
					ebt_print_error("Loop from chain '%s' to chain '%s'",
					   replace->chains[chain_nr]->name,
					   replace->chains[stack[k].chain_nr]->name);
					goto free_stack;
				}
			entries2 = replace->chains[verdict + NF_BR_NUMHOOKS];
			/* check if we've dealt with this chain already */
			if (entries2->hook_mask & (1<<i))
				goto letscontinue;
			entries2->hook_mask |= entries->hook_mask & ~(1 << NF_BR_NUMHOOKS);
			/* Jump to the chain, make sure we know how to get back */
			stack[sp].chain_nr = chain_nr;
			stack[sp].n = j;
			stack[sp].entries = entries;
			stack[sp].e = e;
			sp++;
			j = -1;
			e = entries2->entries->next;
			chain_nr = verdict + NF_BR_NUMHOOKS;
			entries = entries2;
			continue;
letscontinue:
			e = e->next;
		}
		/* We are at the end of a standard chain */
		if (sp == 0)
			continue;
		/* Go back to the chain one level higher */
		sp--;
		j = stack[sp].n;
		chain_nr = stack[sp].chain_nr;
		e = stack[sp].e;
		entries = stack[sp].entries;
		goto letscontinue;
	}
free_stack:
	free(stack);
	return;
}

/* The user will use the match, so put it in new_entry. The ebt_u_match
 * pointer is put in the ebt_entry_match pointer. ebt_add_rule will
 * fill in the final value for new->m. Unless the rule is added to a chain,
 * the pointer will keep pointing to the ebt_u_match (until the new_entry
 * is freed). I know, I should use a union for these 2 pointer types... */
void ebt_add_match(struct ebt_u_entry *new_entry, struct ebt_u_match *m)
{
	struct ebt_u_match_list **m_list, *new;

	for (m_list = &new_entry->m_list; *m_list; m_list = &(*m_list)->next);
	new = (struct ebt_u_match_list *)
	   malloc(sizeof(struct ebt_u_match_list));
	if (!new)
		ebt_print_memory();
	*m_list = new;
	new->next = NULL;
	new->m = (struct ebt_entry_match *)m;
}


        /*
*******************
*******************
**OTHER*FUNCTIONS**
*******************
*******************
         */


/* type = 0 => update chain jumps
 * type = 1 => check for reference, print error when referenced
 * type = 2 => check for reference, don't print error when referenced
 *
 * Returns 1 when type == 1 and the chain is referenced
 * returns 0 otherwise */
static int iterate_entries(struct ebt_u_replace *replace, int type)
{
	int i, j, chain_nr = replace->selected_chain - NF_BR_NUMHOOKS;
	struct ebt_u_entries *entries;
	struct ebt_u_entry *e;

	if (chain_nr < 0)
		ebt_print_bug("iterate_entries: udc = %d < 0", chain_nr);
	for (i = 0; i < replace->num_chains; i++) {
		if (!(entries = replace->chains[i]))
			continue;
		e = entries->entries->next;
		for (j = 0; j < entries->nentries; j++) {
			int chain_jmp;

			if (strcmp(e->t->u.name, EBT_STANDARD_TARGET)) {
				e = e->next;
				continue;
			}
			chain_jmp = ((struct ebt_standard_target *)e->t)->
				    verdict;
			switch (type) {
			case 1:
			case 2:
			if (chain_jmp == chain_nr) {
				if (type == 2)
					return 1;
				ebt_print_error("Can't delete the chain '%s', it's referenced in chain '%s', rule %d",
				                replace->chains[chain_nr + NF_BR_NUMHOOKS]->name, entries->name, j);
				return 1;
			}
			break;
			case 0:
			/* Adjust the chain jumps when necessary */
			if (chain_jmp > chain_nr)
				((struct ebt_standard_target *)e->t)->verdict--;
			break;
			} /* End switch */
			e = e->next;
		}
	}
	return 0;
}

static void decrease_chain_jumps(struct ebt_u_replace *replace)
{
	iterate_entries(replace, 0);
}

/* Used in initialization code of modules */
void ebt_register_match(struct ebt_u_match *m)
{
	int size = EBT_ALIGN(m->size) + sizeof(struct ebt_entry_match);
	struct ebt_u_match **i;

	m->m = (struct ebt_entry_match *)malloc(size);
	if (!m->m)
		ebt_print_memory();
	strcpy(m->m->u.name, m->name);
	m->m->match_size = EBT_ALIGN(m->size);
	m->init(m->m);

	for (i = &ebt_matches; *i; i = &((*i)->next));
	m->next = NULL;
	*i = m;
}

void ebt_register_target(struct ebt_u_target *t)
{
	int size = EBT_ALIGN(t->size) + sizeof(struct ebt_entry_target);
	struct ebt_u_target **i;

	t->t = (struct ebt_entry_target *)malloc(size);
	if (!t->t)
		ebt_print_memory();
	strcpy(t->t->u.name, t->name);
	t->t->target_size = EBT_ALIGN(t->size);
	t->init(t->t);

	for (i = &ebt_targets; *i; i = &((*i)->next));
	t->next = NULL;
	*i = t;
}

void ebt_register_table(struct ebt_u_table *t)
{
	t->next = ebt_tables;
	ebt_tables = t;
}

void ebt_iterate_matches(void (*f)(struct ebt_u_match *))
{
	struct ebt_u_match *i;

	for (i = ebt_matches; i; i = i->next)
		f(i);
}

void ebt_iterate_targets(void (*f)(struct ebt_u_target *))
{
	struct ebt_u_target *i;

	for (i = ebt_targets; i; i = i->next)
		f(i);
}

/* Don't use this function, use ebt_print_bug() */
void __ebt_print_bug(char *file, int line, char *format, ...)
{
	va_list l;

	va_start(l, format);
	fprintf(stderr, "ebtables: %s:%d:--BUG--: \n", file, line);
	vfprintf(stderr, format, l);
	fprintf(stderr, "\n");
	va_end(l);
	exit (-1);
}

/* Don't use this function, use ebt_print_error() */
void __ebt_print_error(char *format, ...)
{
	va_list l;

	va_start(l, format);
	vfprintf(stderr, format, l);
	fprintf(stderr, ".\n");
	va_end(l);
	exit (-1);
}
