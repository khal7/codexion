#include <stdio.h>
#include <stdlib.h>



typedef struct s_test
{
	int *arr;
	int current_n;
	int	max_capacity;
} t_test;


void insert(t_test *arr, int n)
{
	
}

void	ft_swap(int *a, int *b)
{
	int tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

void	heapify_up(t_test *heap, int index)
{

	while (index)
	{
		if (heap->arr[index] < heap->arr[(index - 1 ) / 2])
		{
			ft_swap(&heap->arr[index], &heap->arr[(index - 1 ) / 2]);
			index = (index - 1) / 2;
		}
		else
			break;
	}
}

void	heap_push(t_test *heap, int new_n)
{
	if (heap->current_n >= heap->max_capacity)
		return ;
	
	heap->arr[heap->current_n] = new_n;
	heap->current_n++;
	heapify_up(heap, heap->current_n - 1);	
}

void heapify_down(t_test *heap, int index)
{
	int	left;
	int right;
	int	small;

	while (1)
	{
		left = 2 * index + 1;
		right = 2 * index + 2;
		if (left >= heap->current_n)
			break ;
		small = left;

		if (right < heap->current_n && heap->arr[right] <= heap->arr[left])
			small = right;
		if (heap->arr[index] <= heap->arr[small])
			break ;
		ft_swap(&heap->arr[index], &heap->arr[small]);
		index = small;

	}

}

int heap_pop(t_test *heap)
{
	int res;

	res = heap->arr[0];
	heap->current_n--;
	heap->arr[0] = heap->arr[heap->current_n];
	heapify_down(heap, 0);
	return (res);
}

int main(int ac, char **av)
{
	t_test *my_arr;

	my_arr = malloc(sizeof(t_test));
	my_arr->arr = malloc(256 * sizeof(int));
	my_arr->current_n = 0;
	my_arr->max_capacity = 256;

	for (int i = 0;i < 5;i++)
	{
		my_arr->arr[i] = i + 2;
		my_arr->current_n++;
		
	}
	insert(my_arr, 1);
	printf("\n");


}