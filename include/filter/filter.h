
typedef struct _Filter{
	float alpha;
	int current_value;
} Filter;

void init_filter(Filter *filter);

int update_filter(Filter *filter, int value);
