#include "ft_printf.h"
#include <stdio.h>
#include <limits.h>

static void	run_test(const char *label, int ret_ft, int ret_std)
{
	printf("\n  [%s] ft=%d std=%d %s\n",
		label, ret_ft, ret_std,
		(ret_ft == ret_std) ? "OK" : "KO <<<");
}

int	main(void)
{
	int	a;
	int	b;

	printf("==== %%c ====\n");
	a = ft_printf("ft : [%c]\n", 'A');
	b = printf("std: [%c]\n", 'A');
	run_test("char", a, b);

	a = ft_printf("ft : [%c]\n", 0);
	b = printf("std: [%c]\n", 0);
	run_test("char null", a, b);

	printf("\n==== %%s ====\n");
	a = ft_printf("ft : [%s]\n", "hello");
	b = printf("std: [%s]\n", "hello");
	run_test("string", a, b);

	a = ft_printf("ft : [%s]\n", "");
	b = printf("std: [%s]\n", "");
	run_test("empty string", a, b);

	a = ft_printf("ft : [%s]\n", (char *)NULL);
	b = printf("std: [%s]\n", (char *)NULL);
	run_test("NULL string", a, b);

	printf("\n==== %%d / %%i ====\n");
	a = ft_printf("ft : [%d] [%i]\n", 42, -42);
	b = printf("std: [%d] [%i]\n", 42, -42);
	run_test("d/i basic", a, b);

	a = ft_printf("ft : [%d]\n", 0);
	b = printf("std: [%d]\n", 0);
	run_test("d zero", a, b);

	a = ft_printf("ft : [%d]\n", INT_MAX);
	b = printf("std: [%d]\n", INT_MAX);
	run_test("INT_MAX", a, b);

	a = ft_printf("ft : [%d]\n", INT_MIN);
	b = printf("std: [%d]\n", INT_MIN);
	run_test("INT_MIN", a, b);

	printf("\n==== %%u ====\n");
	a = ft_printf("ft : [%u]\n", 42u);
	b = printf("std: [%u]\n", 42u);
	run_test("u basic", a, b);

	a = ft_printf("ft : [%u]\n", UINT_MAX);
	b = printf("std: [%u]\n", UINT_MAX);
	run_test("UINT_MAX", a, b);

	a = ft_printf("ft : [%u]\n", 0u);
	b = printf("std: [%u]\n", 0u);
	run_test("u zero", a, b);

	printf("\n==== %%x / %%X ====\n");
	a = ft_printf("ft : [%x] [%X]\n", 255u, 255u);
	b = printf("std: [%x] [%X]\n", 255u, 255u);
	run_test("hex 255", a, b);

	a = ft_printf("ft : [%x]\n", 0u);
	b = printf("std: [%x]\n", 0u);
	run_test("hex zero", a, b);

	a = ft_printf("ft : [%x] [%X]\n", UINT_MAX, UINT_MAX);
	b = printf("std: [%x] [%X]\n", UINT_MAX, UINT_MAX);
	run_test("hex UINT_MAX", a, b);

	printf("\n==== %%p ====\n");
	a = ft_printf("ft : [%p]\n", &a);
	b = printf("std: [%p]\n", &a);
	run_test("ptr local", a, b);

	a = ft_printf("ft : [%p]\n", (void *)NULL);
	b = printf("std: [%p]\n", (void *)NULL);
	run_test("ptr NULL", a, b);

	a = ft_printf("ft : [%p]\n", (void *)0x42);
	b = printf("std: [%p]\n", (void *)0x42);
	run_test("ptr 0x42", a, b);

	printf("\n==== %%%% ====\n");
	a = ft_printf("ft : [%%]\n");
	b = printf("std: [%%]\n");
	run_test("percent", a, b);

	printf("\n==== mixed ====\n");
	a = ft_printf("ft : %s = %d (0x%x), ptr=%p, %c%%\n",
				  "answer", 42, 42, &a, 'Z');
	b = printf("std: %s = %d (0x%x), ptr=%p, %c%%\n",
			   "answer", 42, 42, &a, 'Z');
	run_test("mixed", a, b);

	printf("\n==== format vide / juste texte ====\n");
	a = ft_printf("");
	b = printf("");
	run_test("empty format", a, b);

	a = ft_printf("juste du texte\n");
	b = printf("juste du texte\n");
	run_test("plain text", a, b);

	return (0);
}
