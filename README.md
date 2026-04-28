*This activity has been created as part of the 42 curriculum by ahenry.*

# ft_printf

## Description

`ft_printf` est une réimplémentation de la fonction standard `printf` du langage C. Le but du projet est de recoder cette fonction emblématique afin de comprendre en profondeur le fonctionnement des fonctions à arguments variables (`stdarg.h`), la gestion du formatage de sortie, ainsi que la conversion de différents types de données vers leur représentation textuelle.

La bibliothèque produite, `libftprintf.a`, expose une fonction `ft_printf` qui se comporte comme la `printf` originale, à l'exception de la gestion des `buffer` et des flags étendus. Elle prend en charge les conversions suivantes :

| Conversion | Type attendu          | Description                                   |
|------------|-----------------------|-----------------------------------------------|
| `%c`       | `int`                 | Affiche un caractère                          |
| `%s`       | `char *`              | Affiche une chaîne (`(null)` si `NULL`)       |
| `%p`       | `void *`              | Affiche une adresse en hexadécimal préfixée   |
| `%d` / `%i`| `int`                 | Affiche un entier signé en base 10            |
| `%u`       | `unsigned int`        | Affiche un entier non signé en base 10        |
| `%x`       | `unsigned int`        | Affiche un entier en hexadécimal minuscule    |
| `%X`       | `unsigned int`        | Affiche un entier en hexadécimal majuscule    |
| `%%`       | —                     | Affiche un caractère `%` littéral             |

La valeur de retour est le nombre de caractères effectivement écrits sur la sortie standard, ou `-1` en cas d'erreur.

## Instructions

### Prérequis

- Un compilateur C compatible (`cc` / `gcc` / `clang`)
- `make`
- La bibliothèque `libft.a` (avec son `libft.h`) accessible à la racine du projet, puisque `ft_printf` réutilise notamment `ft_itoa` et `ft_strlen`.

### Compilation

À la racine du projet :

```sh
make
```

Cette commande génère l'archive statique `libftprintf.a`. Le `Makefile` recopie d'abord `libft.a` puis y ajoute les objets de `ft_printf`, de sorte que l'archive finale est auto-suffisante au moment de la liaison.

Règles disponibles :

| Règle    | Effet                                                       |
|----------|-------------------------------------------------------------|
| `all`    | Compile `libftprintf.a` (règle par défaut)                  |
| `clean`  | Supprime les fichiers objets `.o`                           |
| `fclean` | Supprime les `.o` ainsi que `libftprintf.a`                 |
| `re`     | Équivalent à `fclean` puis `all`                            |

### Utilisation

Inclure le header puis lier la bibliothèque lors de la compilation de votre programme :

```c
#include "ft_printf.h"

int main(void)
{
    int n;

    n = ft_printf("Hello %s, you are %d years old (ptr: %p)\n",
                  "world", 42, &main);
    ft_printf("%d caractères écrits\n", n);
    return (0);
}
```

Compilation de l'exemple :

```sh
cc main.c -L. -lftprintf -o demo
./demo
```

## Algorithme et structure de données

### Choix algorithmique

Le cœur de `ft_printf` repose sur un **parseur en une seule passe** sur la chaîne de format. On parcourt `format` caractère par caractère ; dès qu'un `%` est rencontré et qu'il est suivi d'un caractère, on délègue le traitement à une fonction de **dispatch** (`ft_dispatch`) qui sélectionne le bon handler selon le specifier.

Ce choix présente plusieurs avantages :

- **Complexité linéaire `O(n)`** par rapport à la longueur de la chaîne de format, sans backtracking.
- **Séparation des responsabilités** : `ft_printf` ne s'occupe que du parsing, chaque type a son propre fichier (`ft_print_char.c`, `ft_print_str.c`, etc.). Cela respecte la Norme 42 (≤ 25 lignes / fonction, ≤ 5 fonctions / fichier) et facilite l'ajout futur de nouveaux specifiers.
- **Propagation explicite du compteur** : chaque handler retourne le nombre de caractères qu'il a écrits, ce qui permet à `ft_printf` d'agréger le total exact, conformément au comportement de la `printf` standard.
- **Gestion d'erreur centralisée** : un retour `-1` d'un handler interrompt immédiatement l'exécution et libère proprement le `va_list` via `va_end` avant de remonter l'erreur à l'appelant.

### Conversion des entiers : récursivité plutôt qu'itération

Pour `%u`, `%x`, `%X` et `%p`, l'écriture des chiffres se fait par **récursivité descendante** : on divise par la base (10 ou 16), on rappelle la fonction sur le quotient, puis on écrit le chiffre courant. Ce schéma évite d'avoir à allouer un buffer temporaire et à inverser les caractères, comme on le ferait avec une approche itérative classique.

```c
if (n >= 10)
    count += ft_print_unsigned(n / 10);
c = (n % 10) + '0';
write(1, &c, 1);
```

La profondeur de récursion est bornée par le nombre de chiffres de l'entier (au plus 10 pour un `unsigned int` en base 10, 16 pour un `unsigned long` en base 16), donc le coût en pile est négligeable et constant.

Pour `%d` / `%i`, on s'appuie sur `ft_itoa` (issue de `libft`) afin de gérer correctement le cas particulier de `INT_MIN`, dont la valeur absolue ne tient pas dans un `int`. Cette délégation évite de réécrire une logique déjà testée et conforme dans `libft`.

### Pourquoi pas de table de dispatch ?

Une alternative aurait été d'utiliser un tableau de pointeurs de fonctions indexé par le caractère du specifier. Elle a été écartée pour deux raisons :

1. Les handlers ne partagent **pas la même signature** (`ft_print_hex` prend la base en paramètre supplémentaire, `ft_print_ptr` reçoit un `void *`, etc.), ce qui obligerait à uniformiser artificiellement les prototypes.
2. La cascade de `if` actuelle est **plus lisible**, **type-safe** et reste de complexité `O(1)` puisque le nombre de specifiers est petit et fixe.

### Structures de données

Aucune structure de données dynamique n'est nécessaire :

- Les arguments variables sont consommés à la volée via une `va_list` (`stdarg.h`), ce qui évite tout stockage intermédiaire.
- L'écriture est faite directement vers `stdout` avec `write(1, ...)`, sans buffer interne. Cela simplifie le code et évite de dupliquer la logique de bufferisation que `printf` standard implémente déjà au-dessus du syscall.
- Le seul allocation dynamique se produit dans `ft_itoa`, et le buffer retourné est libéré immédiatement après l'écriture pour garantir l'absence de fuite mémoire.

## Resources

### Documentation et références

- `man 3 printf` — comportement de référence à reproduire.
- `man 3 stdarg` — manipulation des arguments variadiques (`va_start`, `va_arg`, `va_end`).
- *The C Programming Language* (Kernighan & Ritchie), chapitre 7 sur les entrées/sorties formatées.
- [cppreference — printf](https://en.cppreference.com/w/c/io/fprintf) — table des conversions et de leurs sémantiques.
- Norme 42 — contraintes de style et de structure imposées par l'école.