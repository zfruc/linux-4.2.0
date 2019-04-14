/*
*  defined for fake I/O interfaces
*/
struct throtl_qnode {
	struct list_head	node;		/* service_queue->queued[] */
	struct bio_list		bios;		/* queued bios */
	struct throtl_grp	*tg;		/* tg this qnode belongs to */
};


struct throtl_service_queue {
	struct throtl_service_queue *parent_sq;	/* the parent service_queue */

	/*
	 * Bios queued directly to this service_queue or dispatched from
	 * children throtl_grp's.
	 */
	struct list_head	queued[2];	/* throtl_qnode [READ/WRITE] */
	unsigned int		nr_queued[2];	/* number of queued bios */

	/*
	 * RB tree of active children throtl_grp's, which are sorted by
	 * their ->disptime.
	 */
	struct rb_root		pending_tree;	/* RB tree of active tgs */
	struct rb_node		*first_pending;	/* first node in the tree */
	unsigned int		nr_pending;	/* # queued in the tree */
	unsigned long		first_pending_disptime;	/* disptime of the first tg */
	struct timer_list	pending_timer;	/* fires on first_pending_disptime */
};

enum tg_state_flags {
	THROTL_TG_PENDING	= 1 << 0,	/* on parent's pending tree */
	THROTL_TG_WAS_EMPTY	= 1 << 1,	/* bio_lists[] became non-empty */
};

struct throtl_grp {
	/* must be the first member */
	struct blkg_policy_data pd;

	/* active throtl group service_queue member */
	struct rb_node rb_node;

	/* throtl_data this group belongs to */
	struct throtl_data *td;

	/* this group's service queue */
	struct throtl_service_queue service_queue;

	/*
	 * qnode_on_self is used when bios are directly queued to this
	 * throtl_grp so that local bios compete fairly with bios
	 * dispatched from children.  qnode_on_parent is used when bios are
	 * dispatched from this throtl_grp into its parent and will compete
	 * with the sibling qnode_on_parents and the parent's
	 * qnode_on_self.
	 */
	struct throtl_qnode qnode_on_self[2];
	struct throtl_qnode qnode_on_parent[2];

	/*
	 * Dispatch time in jiffies. This is the estimated time when group
	 * will unthrottle and is ready to dispatch more bio. It is used as
	 * key to sort active groups in service tree.
	 */
	unsigned long disptime;

	unsigned int flags;

	/* are there any throtl rules between this group and td? */
	bool has_rules[3];

	/* bytes per second rate limits */
	uint64_t bps[3];

	/* IOPS limits */
	unsigned int iops[3];

	/* Number of bytes disptached in current slice */
	uint64_t bytes_disp[3];
	/* Number of bio's dispatched in current slice */
	unsigned int io_disp[3];

	/* When did we start a new slice */
	unsigned long slice_start[3];
	unsigned long slice_end[3];

	/* Per cpu stats pointer */
	struct tg_stats_cpu __percpu *stats_cpu;

	/* List of tgs waiting for per cpu stats memory to be allocated */
	struct list_head stats_alloc_node;
};




extern struct throtl_grp *sq_to_tg(struct throtl_service_queue *sq);
extern void throtl_trim_slice(struct throtl_grp *tg, bool rw);
extern void throtl_update_dispatch_stats(struct blkcg_gq *blkg, u64 bytes,int rw);
extern struct throtl_grp *throtl_lookup_create_tg(struct throtl_data *td,struct blkcg *blkcg);
extern struct throtl_grp *throtl_lookup_tg(struct throtl_data *td,struct blkcg *blkcg);
extern struct blkcg_gq *tg_to_blkg(struct throtl_grp *tg);
