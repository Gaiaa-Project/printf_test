*This activity has been created as part of the 42 curriculum by ahenry.*

# get_next_line

## Description

`get_next_line` est une fonction C qui retourne, ligne par ligne, le contenu d'un fichier (ou de tout autre flux) à partir d'un simple **file descriptor**. Chaque appel successif à `get_next_line(fd)` renvoie la ligne suivante, terminateur `\n` inclus, jusqu'à atteindre la fin du fichier (ou une erreur), où la fonction renvoie `NULL`.

L'enjeu pédagogique du projet est triple :

1. Comprendre et exploiter les **variables statiques** pour préserver de l'état entre des appels successifs sans recourir à des variables globales (interdites par le sujet).
2. Maîtriser la **gestion mémoire dynamique** (`malloc` / `free`) avec une discipline rigoureuse, sans aucune fuite quel que soit le scénario d'erreur.
3. Implémenter un **buffering manuel** au-dessus du syscall `read`, robuste pour toute valeur de `BUFFER_SIZE` (1, 42, 9999, voire 10000000).

```c
char	*get_next_line(int fd);
```

| Retour | Signification                                                      |
|--------|--------------------------------------------------------------------|
| `char *` non `NULL` | Ligne lue, terminée par `\n` (sauf en fin de fichier sans saut de ligne final) |
| `NULL`              | Fin de fichier atteinte ou erreur (`read` < 0, `malloc` échoué, `fd` invalide) |

## Instructions

### Prérequis

- Un compilateur C compatible (`cc` / `gcc` / `clang`)
- Aucune dépendance externe : la `libft` est explicitement **interdite** dans ce projet, toutes les fonctions auxiliaires sont réimplémentées localement dans `get_next_line_utils.c`.

### Compilation

Les fichiers sont à la racine du dépôt. La taille du buffer interne utilisé par `read` est paramétrable au moment de la compilation via la macro `BUFFER_SIZE` :

```sh
cc -Wall -Wextra -Werror -D BUFFER_SIZE=42 get_next_line.c get_next_line_utils.c <votre_main>.c
```

Si l'option `-D BUFFER_SIZE` est omise, la valeur par défaut est `42` (définie dans `get_next_line.h`).

### Utilisation

```c
#include "get_next_line.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

int	main(void)
{
	int		fd;
	char	*line;

	fd = open("fichier.txt", O_RDONLY);
	if (fd < 0)
		return (1);
	line = get_next_line(fd);
	while (line != NULL)
	{
		printf("%s", line);
		free(line);
		line = get_next_line(fd);
	}
	close(fd);
	return (0);
}
```

La fonction fonctionne aussi sur l'**entrée standard** (`fd = 0`), ce qui permet de la tester en pipe :

```sh
echo -e "ligne1\nligne2\nligne3" | ./mon_programme
```

⚠️ **Comportement indéfini** (conformément au sujet) :
- Si le fichier pointé par `fd` change entre deux appels alors que `read` n'a pas encore atteint la fin du fichier.
- Sur un fichier binaire.

## Algorithme et structure de données

### Vue d'ensemble

L'algorithme repose sur **quatre étapes** orchestrées par `get_next_line` :

```
┌──────────────────────────────────────────────────────────────────────┐
│  static stash  ─────►  read_to_stash  ─────►  extract_line  ─────►  │
│                                                      │               │
│                                                      ▼               │
│                                              update_stash  ──► next  │
└──────────────────────────────────────────────────────────────────────┘
```

1. **`read_to_stash`** lit dans le `fd` par blocs de `BUFFER_SIZE` octets et accumule les données dans une chaîne persistante (la *stash*) **jusqu'à rencontrer un `\n`** ou la fin du fichier.
2. **`extract_line`** extrait depuis la stash la portion correspondant à la prochaine ligne (jusqu'au premier `\n` inclus, ou jusqu'à la fin si aucun `\n`).
3. **`update_stash`** conserve dans la stash uniquement le **reliquat** situé après le `\n`, qui servira pour les appels suivants.
4. La ligne extraite est retournée à l'appelant ; la stash est conservée d'un appel sur l'autre grâce à la variable statique.

### Pourquoi une variable statique

Le sujet interdit explicitement les variables globales mais autorise les statiques locales. La variable :

```c
static char	*stash;
```

déclarée à l'intérieur de `get_next_line` est :

- **Persistante** entre les appels (durée de vie du programme).
- **Locale** au scope de la fonction (encapsulation, pas de pollution globale).
- **Initialisée à `NULL`** automatiquement (durée de stockage statique).

C'est la pierre angulaire du projet : sans elle, impossible de mémoriser le surplus lu mais non encore consommé d'un appel à l'autre.

### Pourquoi lire « le moins possible »

Le sujet recommande explicitement de ne **pas lire tout le fichier d'un coup** mais de s'arrêter dès qu'un `\n` est disponible. La condition de boucle de `read_to_stash` reflète cette exigence :

```c
while (bytes_read > 0 && ft_strchr_gnl(stash, '\n') == NULL)
```

Conséquences :

- On ne consomme pas plus de données que nécessaire pour rendre la prochaine ligne.
- Le coût mémoire reste proportionnel à la **plus longue ligne** rencontrée et non à la taille du fichier.
- La fonction supporte naturellement les flux infinis (`stdin`, pipes, sockets).

### Robustesse face à `BUFFER_SIZE`

| `BUFFER_SIZE` | Comportement                                                         |
|---------------|----------------------------------------------------------------------|
| `1`           | Une lecture par caractère, plus lent mais correct.                   |
| `42`          | Cas nominal du sujet.                                                |
| `9999`        | Une seule lecture suffit pour la plupart des fichiers texte.         |
| `10000000`    | Allocation conséquente du buffer interne, mais l'algorithme tient.   |

L'algorithme est **invariant à la taille du buffer** : la stash absorbe les blocs de la taille demandée, et la détection du `\n` se fait sur la stash globale, pas sur le buffer.

### Choix de la stratégie « *consuming join* »

`ft_strjoin_gnl` a été conçu avec une sémantique particulière : il **libère son premier argument** une fois la concaténation effectuée.

```c
char	*ft_strjoin_gnl(char *s1, const char *s2);   /* free(s1) en interne */
```

Avantage : le code appelant reste linéaire et sans variable temporaire :

```c
stash = ft_strjoin_gnl(stash, buffer);
if (stash == NULL)
	return (free(buffer), NULL);
```

Plutôt que :

```c
tmp = ft_strjoin(stash, buffer);
free(stash);
stash = tmp;
if (stash == NULL) ...
```

Cela réduit le risque de fuite (une seule responsabilité : la stash *est* l'objet vivant) et garde les fonctions sous les **25 lignes** imposées par la Norme 42.

### Structures de données

Aucune structure complexe — uniquement deux chaînes C :

| Variable        | Portée                       | Rôle                                                       |
|-----------------|------------------------------|------------------------------------------------------------|
| `stash`         | `static` dans `get_next_line` | Reliquat persistant entre les appels                       |
| `buffer`        | locale à `read_to_stash`     | Tampon de lecture brute pour `read`, taille `BUFFER_SIZE+1` |

Le choix d'une chaîne plate (plutôt que d'une liste chaînée de chunks comme on le fait parfois pour la version *bonus*) est volontaire :

- **Simplicité** : pas de gestion de nœuds, pas d'aplatissement final.
- **Suffisant pour la version mandatory** : un seul `fd` à gérer, l'amortissement du `realloc`-via-`strjoin` reste acceptable pour les tailles de lignes habituelles.
- **Lisibilité** : trois fonctions courtes (`read_to_stash`, `extract_line`, `update_stash`) suffisent à exprimer toute la logique.

### Gestion d'erreurs

Toutes les voies d'échec libèrent la stash et retournent `NULL` :

| Cas d'échec                  | Action                                       |
|------------------------------|----------------------------------------------|
| `fd < 0` ou `BUFFER_SIZE <= 0` | Retour immédiat `NULL`                     |
| `malloc` du buffer échoue    | `free(stash)` puis `NULL`                    |
| `read` retourne `-1`         | `free(buffer)` + `free(stash)` puis `NULL`   |
| `malloc` dans le join échoue | `free(buffer)` puis `NULL` (s1 déjà free dans le join) |
| Fin de fichier sans contenu  | `free(line)` (ligne vide) puis `NULL`        |

### Complexité

Pour une ligne de longueur `L` lue avec un buffer de taille `B` :

- **Temps** : `O(L)` amorti — chaque caractère est copié un nombre constant de fois entre la stash et la ligne extraite.
- **Espace** : `O(L + B)` — la stash contient au plus la ligne courante plus un bloc en cours, le buffer de lecture pèse `B`.

## Resources

### Documentation et références

- `man 2 read` — sémantique du syscall et codes de retour (`-1` / `0` / `> 0`).
- `man 3 malloc` et `man 3 free` — gestion mémoire dynamique.
- `man 7 fcntl` et `man 2 open` — ouverture de fichiers pour les tests.
- *The C Programming Language* (Kernighan & Ritchie), chapitre 7 — buffering et I/O bas niveau.
- [cppreference — Storage class specifiers](https://en.cppreference.com/w/c/language/storage_duration) — détails sur `static` et la durée de stockage statique.
- [Linus Akesson — A heap of static](https://www.linusakesson.net/programming/) — articles divers sur la programmation système C.
- Norme 42 — contraintes de style et de structure (≤ 25 lignes / fonction, ≤ 5 fonctions / fichier, déclarations en tête de bloc, parenthèses sur les `return`...).

### Articles et tutoriels

- [42-cursus-tips: get_next_line](https://github.com/JCluzet/42_BestCheatSheet) — patterns classiques rencontrés dans le projet.
- [gnlTester](https://github.com/Tripouille/gnlTester) — banc de tests communautaire (cas extrêmes, fuites, BUFFER_SIZE variables).
- Articles sur la sémantique de `static` en C (storage class specifiers) pour bien comprendre la persistance entre appels.

---

*Dernière mise à jour : 2026-04-28*
