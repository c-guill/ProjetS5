#include <math.h>
#include <stdio.h>

#define MAX_NODES 64

typedef struct {
	int T[MAX_NODES];
	int n;
} ARRAY;

typedef struct {
	int F[MAX_NODES];
	int n;
} HEAP;

//
//------------------------------------------------------------------------------
//

HEAP CreateHeap(void)
{
	HEAP h;

	h.n = 0;

	return h;
}

//
//------------------------------------------------------------------------------
//

void InsertHeap(HEAP *h, int e)
{
	int p, i, tmp;

	h->n++;
	h->F[h->n] = e;
	p = (int)h->n/2;
	i = h->n;

	while (i != 1 && h->F[p] < h->F[i])
	{
		tmp = h->F[i];
		h->F[i] = h->F[p];
		h->F[p] = tmp;
		i = p;
		p = (int)i/2;
	}

	return;
}

//
//------------------------------------------------------------------------------
//

void ExtractMaxHeap(HEAP *h)
{
	int i, m, tmp;

	tmp = h->F[h->n];
	h->F[h->n] = h->F[1];
	h->F[1] = tmp;
	h->n--;
	i = 1;

	while ((2*i <= h->n && h->F[i] < h->F[2*i]) || (2*i+1 <= h->n && h->F[i] < h->F[2*i+1]))
	{
		if (2*i+1 > h->n)
		{
			m = 2*i;
		}
		else
		{
			if (h->F[2*i] >= h->F[2*i+1])
			{
				m = 2*i;
			}
			else
			{
				m = 2*i+1;
			}
		}
		tmp = h->F[m];
		h->F[m] = h->F[i];
		h->F[i]= tmp;
		i = m;
	}

	return;
}

//
//------------------------------------------------------------------------------
//

void PrintHeap(HEAP *h)
{
	int i;

	for (i = 1; i <= h->n; i++)
	{
		printf("%d ", h->F[i]);
	}

	printf("\n");

	return;
}

//
//------------------------------------------------------------------------------
//

void PrintArray(ARRAY *a)
{
	int i;

	for (i = 1; i <= a->n; i++)
	{
		printf("%d ", a->T[i]);
	}

	printf("\n");

	return;
}

//
//------------------------------------------------------------------------------
//

void ArrayToHeap(ARRAY *a)
{
	int k, N, p, i, tmp;

	N = a->n;

	for (k = 1; k <= N; k++)
	{
		p = (int)k/2;
		i = k;

		while (i != 1 && a->T[p] < a->T[i])
		{
			tmp = a->T[i];
			a->T[i] = a->T[p];
			a->T[p] = tmp;
			i = p;
			p = (int)i/2;
		}
	}

	return;
}

//
//------------------------------------------------------------------------------
//

void Step2(ARRAY *a)
{
	int k, N, i, m, tmp;

	N = a->n;
	k = N;

	while (k > 1)
	{
		tmp = a->T[k];
		a->T[k] = a->T[1];
		a->T[1] = tmp;
		k--;
		i = 1;

		while ((2*i <= k && a->T[i] < a->T[2*i]) || (2*i+1 <= k && a->T[i] < a->T[2*i+1]))
		{
			if (2*i+1 > k)
			{
				m = 2*i;
			}
			else
			{
				if (a->T[2*i] >= a->T[2*i+1])
				{
					m = 2*i;
				}
				else
				{
					m = 2*i+1;
				}
			}
			tmp = a->T[m];
			a->T[m] = a->T[i];
			a->T[i]= tmp;
			i = m;
		}
	}

	return;
}

//
//------------------------------------------------------------------------------
//

int main(void)
{
	printf("%f\n", 2 * pow(3, 2));

	ARRAY tab = {{0, 11, 2, 18, 6, 15, 20, 9, 16, 5}, 9};

	PrintArray(&tab);
	ArrayToHeap(&tab);
	PrintArray(&tab);
	Step2(&tab);
	PrintArray(&tab);


	HEAP h;

	h = CreateHeap();

	InsertHeap(&h, 11);
	InsertHeap(&h, 2);
	InsertHeap(&h, 18);
	InsertHeap(&h, 6);
	InsertHeap(&h, 15);
	InsertHeap(&h, 20);
	InsertHeap(&h, 9);
	InsertHeap(&h, 16);
	InsertHeap(&h, 5);

	PrintHeap(&h);

	ExtractMaxHeap(&h);

	PrintHeap(&h);

	ExtractMaxHeap(&h);

	PrintHeap(&h);

	return 0;
}