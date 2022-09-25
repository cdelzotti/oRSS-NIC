#define HAVE_STDATOMIC_H 1

struct ovs_cmdl_context;
struct ofputil_flow_stats;

void custom_dump_flows(char *dump_specs, struct ofputil_flow_stats **fses, size_t *n_fses);
void free_dump(struct ofputil_flow_stats *fses, size_t n_fses);
void custom_del_flows();
void custom_add_flow(char *flow_spec);