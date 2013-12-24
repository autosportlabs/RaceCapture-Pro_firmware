
typedef struct _Filter{
	float alpha;
	int current_value;
} Filter;

void init_filter(Filter *filter, float alpha);

int update_filter(Filter *filter, int value);
