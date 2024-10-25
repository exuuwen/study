libibverbs/device.c
providers/rxe/rxe.c

1. ibv_get_device_list:

struct verbs_sysfs_dev {
        char sysfs_name[IBV_SYSFS_NAME_MAX]; //uverbs0
        dev_t sysfs_cdev;    //uverbs cdev
        char ibdev_name[IBV_SYSFS_NAME_MAX]; // rxe0
        char ibdev_path[IBV_SYSFS_PATH_MAX]; /sys/class/infiniband/rxe0
        uint64_t node_guid;
        uint32_t driver_id;
        enum ibv_node_type node_type;
        int ibdev_idx;
        uint32_t num_ports;
        uint32_t abi_ver;
};


struct rxe_device {
	struct verbs_device	ibv_dev;
	int	abi_version;
};

struct verbs_device {
	struct ibv_device device; /* Must be first */
	const struct verbs_device_ops *ops;   ---> rxe_dev_ops
	struct verbs_sysfs_dev *sysfs;
};

struct ibv_device {
	struct _ibv_device_ops	_ops;
	enum ibv_node_type	node_type;
	enum ibv_transport_type	transport_type;
	char			name[IBV_SYSFS_NAME_MAX]; // rxe0
	char			dev_name[IBV_SYSFS_NAME_MAX]; // uverbs0
	/* Path to infiniband_verbs class device in sysfs */
	char			dev_path[IBV_SYSFS_PATH_MAX]; // /sys/class/infiniband_verbs/uverbs0
	/* Path to infiniband class device in sysfs */
	char			ibdev_path[IBV_SYSFS_PATH_MAX]; // /sys/class/infiniband/rxe0
};


uapi

struct ib_uverbs_attr {
	__u16 attr_id;		/* command specific type attribute */
	__u16 len;		/* only for pointers and IDRs array */
	__u16 flags;		/* combination of UVERBS_ATTR_F_XXXX */
	union {
		struct {
			__u8 elem_id;
			__u8 reserved;
		} enum_data;
		__u16 reserved;
	} attr_data;
	union {
		/*
		 * ptr to command, inline data, idr/fd or
		 * ptr to __u32 array of IDRs
		 */
		__aligned_u64 data;
		/* Used by FD_IN and FD_OUT */
		__s64 data_s64;
	};
};

struct ib_uverbs_ioctl_hdr {
	__u16 length;
	__u16 object_id;
	__u16 method_id;
	__u16 num_attrs;
	__aligned_u64 reserved1;
	__u32 driver_id;
	__u32 reserved2;
	struct ib_uverbs_attr  attrs[0];
};



2. ibv_open_device ---> verbs_open_device

{
   open_cdev
   verbs_device->ops->alloc_context
}

3. context
   rxe_alloc_context
       ----> DECLARE_FBCMD_BUFFER(cmdb, UVERBS_OBJECT_DEVICE,
			     UVERBS_METHOD_GET_CONTEXT, 2, link);
struct rxe_context {
	struct verbs_context	ibv_ctx;
};

struct verbs_context {
	struct verbs_ex_private *priv;
	struct ibv_context context;	/* Must be last field in the struct */
	//hook                 -----> rxe_ctx_ops
}

struct ibv_context {
	struct ibv_device      *device;
	struct ibv_context_ops	ops;   ---> rxe_ctx_ops
	int			cmd_fd;
	int			async_fd;
};

struct verbs_ex_private {
	uint32_t driver_id;
	bool use_ioctl_write;
	struct verbs_context_ops ops;  ---> rxe_ctx_ops
};




4. ibv_alloc_pd ---> rxe_alloc_pd ----> execute_cmd_write(IB_USER_VERBS_CMD_ALLOC_PD)
	pd->handle  = resp->pd_handle;
	pd->context = context;

struct ibv_pd {
        struct ibv_context     *context;
        uint32_t                handle;
};

struct ib_uverbs_alloc_pd {
        __aligned_u64 response;
};

struct ib_uverbs_alloc_pd_resp {
        __u32 pd_handle;
};


5. ibv_reg_mr ---> rxe_reg_mr ---> execute_cmd_write(IB_USER_VERBS_CMD_REG_MR)

struct ibv_mr {
        struct ibv_context     *context;
        struct ibv_pd          *pd; 
        void                   *addr;
        size_t                  length;
        uint32_t                handle;
        uint32_t                lkey;
        uint32_t                rkey;
};

struct verbs_mr {
	struct ibv_mr		ibv_mr;
	enum ibv_mr_type        mr_type;
	int access;
};

struct ib_uverbs_reg_mr {
        __aligned_u64 response;
        __aligned_u64 start;
        __aligned_u64 length;
        __aligned_u64 hca_va;
        __u32 pd_handle;
        __u32 access_flags;
};

struct ib_uverbs_reg_mr_resp {
        __u32 mr_handle;
        __u32 lkey;
        __u32 rkey;
};

6. rxe_create_cq-->rxe_create_cq
a. ibv_cmd_create_cq -->UVERBS_METHOD_CQ_CREATE
b.   cq->queue = mmap(NULL, resp.mi.size, PROT_READ | PROT_WRITE, MAP_SHARED,
                         context->cmd_fd, resp.mi.offset);

struct ibv_cq {
        struct ibv_context     *context;
        struct ibv_comp_channel *channel;
        void                   *cq_context;
        uint32_t                handle;
        int                     cqe; 

        uint32_t                comp_events_completed;
        uint32_t                async_events_completed;
};

struct verbs_cq {
        union {
                struct ibv_cq cq; 
                struct ibv_cq_ex cq_ex;
        };  
};

struct rxe_cq {
        struct verbs_cq         vcq;
        struct mminfo           mmap_info;
        struct rxe_queue_buf    *queue;
};

7. ibv_create_qp--> rxe_create_qp

struct ibv_qp {
        struct ibv_context     *context;
        void                   *qp_context;
        struct ibv_pd          *pd; 
        struct ibv_cq          *send_cq;
        struct ibv_cq          *recv_cq;
        struct ibv_srq         *srq;
        uint32_t                handle;
        uint32_t                qp_num;
        enum ibv_qp_state       state;
        enum ibv_qp_type        qp_type;

        uint32_t                events_completed;
};


struct verbs_qp {
        union {
                struct ibv_qp qp;
        };
        uint32_t                comp_mask;
};

struct rxe_wq {      
        struct rxe_queue_buf    *queue;
        unsigned int            max_sge;
        unsigned int            max_inline;
};

struct rxe_qp {
        struct verbs_qp         vqp;
        struct mminfo           rq_mmap_info;
        struct rxe_wq           rq;
        struct mminfo           sq_mmap_info;
        struct rxe_wq           sq;
        unsigned int            ssn;

        /* new API support */
        uint32_t                cur_index;
};

8. ibv_modify_qp--->rxe_modify_qp--->execute_cmd_write_req(IB_USER_VERBS_CMD_MODIFY_QP)

struct ib_uverbs_modify_qp {
        struct ib_uverbs_qp_dest dest; 
        struct ib_uverbs_qp_dest alt_dest;
        __u32 qp_handle;
        __u32 attr_mask;
        __u32 qkey;
        __u32 rq_psn;
        __u32 sq_psn;
        __u32 dest_qp_num;
        __u32 qp_access_flags;
        __u16 pkey_index;
        __u16 alt_pkey_index;
        __u8  qp_state;
        __u8  cur_qp_state;
        __u8  path_mtu;
        __u8  path_mig_state;
        __u8  en_sqd_async_notify;
        __u8  max_rd_atomic; 
        __u8  max_dest_rd_atomic;
        __u8  min_rnr_timer; 
        __u8  port_num;
        __u8  timeout;
        __u8  retry_cnt;
        __u8  rnr_retry;
        __u8  alt_port_num;
        __u8  alt_timeout;
};




kernel:

nldev.c
rxe.c   rxe_verbs.c

1. uverbs_device

a. ib_register_device

struct rxe_dev {
        struct ib_device        ib_dev;
        struct ib_device_attr   attr;
};

struct ib_device {
        /* Do not access @dma_device directly from ULP nor from HW drivers. */
        struct device                *dma_device;
        struct ib_device_ops         ops;   ---->      rxe_dev_ops
}


uverbs_main.c  

b. ib_register_client

uverbs_client：
struct ib_client {   
        const char *name;
	xxxxxx;
}

c. add_client_context

ib_uverbs_add_one ib_client + ib_device ---> ib_uverbs_device

struct ib_uverbs_device {
        u32                                     num_comp_vectors;
        struct ib_device        __rcu          *ib_dev;
        int                                     devnum;
        struct cdev                             cdev;
        struct uverbs_api                       *uapi;
};


uverbs_std_types_device.c  uverbs_ioctl.c
uverbs_uapi.c

d. uapi

ib_uverbs_add_one  --> ib_uverbs_create_uapi

def:
struct uverbs_attr_def {
        u16                           id;  
        struct uverbs_attr_spec       attr;
};

struct uverbs_method_def {
        u16                                  id;  
        size_t                               num_attrs;
        const struct uverbs_attr_def * const (*attrs)[];
        int (*handler)(struct uverbs_attr_bundle *attrs);
};

struct uverbs_object_def {
        u16                                      id;  
        const struct uverbs_obj_type            *type_attrs; // const struct uverbs_obj_type_class *
        size_t                                   num_methods;
        const struct uverbs_method_def * const (*methods)[];
};


store to ------->

object_id ---> object
struct uverbs_api_object {
	const struct uverbs_obj_type *type_attrs;
	const struct uverbs_obj_type_class *type_class;
	u8 disabled:1;
	u32 id;
};

object_id + method_id ---> method
struct uverbs_api_ioctl_method {
	int(__rcu *handler)(struct uverbs_attr_bundle *attrs);
	DECLARE_BITMAP(attr_mandatory, UVERBS_API_ATTR_BKEY_LEN);
	u16 bundle_size;
	u8 has_udata:1;
	u8 destroy_bkey;
};

object_id + method_id + attr ---> attr
struct uverbs_api_attr {
	struct uverbs_attr_spec spec;
};


UAPI_DEF_WRITE:

struct uverbs_api_write_method {
        int (*handler)(struct uverbs_attr_bundle *attrs);
        u8 disabled:1;
        u8 is_ex:1;
        u8 has_udata:1;
        u8 has_resp:1;
        u8 req_size;
        u8 resp_size;
};


   object_id                 method_id
UVERBS_OBJECT_DEVICE + UVERBS_METHOD_INVOKE_WRITE
   attr 
UVERBS_ATTR_WRITE_CMD ---> 
cmd_method_key |= uapi_key_write_method(def->write.command_num)



userspace to kernel ioctl
ib_uverbs_attr ---> uverbs_attr


struct uverbs_ptr_attr {
        union {
                void *ptr;
                u64 data;
        };   
        u16             len; 
        u16             uattr_idx;
        u8              enum_id;
};

struct uverbs_obj_attr {
        struct ib_uobject               *uobject;
        const struct uverbs_api_attr    *attr_elm;
};

struct uverbs_objs_arr_attr {
        struct ib_uobject **uobjects;
        u16 len; 
};

struct uverbs_attr {
        union {
                struct uverbs_ptr_attr  ptr_attr; // UVERBS_ATTR_TYPE_ENUM_IN, PTR_IN PTR_OUT
                struct uverbs_obj_attr  obj_attr; //UVERBS_ATTR_TYPE_IDR, FD
                struct uverbs_objs_arr_attr objs_arr_attr; //UVERBS_ATTR_TYPE_IDRS
        };   
};



2. ib_uverbs_open

alloc ib_uverbs_file to ufile->private

struct ib_uverbs_file {
        struct ib_uverbs_device                *device;
        /*  
         * ucontext must be accessed via ib_uverbs_get_ucontext() or with
         * ucontext_lock held
         */
        struct ib_ucontext                     *ucontext;
};


3. context
UVERBS_HANDLER(UVERBS_METHOD_GET_CONTEXT)--->
     ib_alloc_ucontext
     ib_init_ucontext--->ops.alloc_ucontext--->rxe_alloc_ucontext--->rxe_add_to_pool(&rxe->uc_pool, &uc->pelem);

struct rxe_ucontext {
        struct ib_ucontext ibuc;
        struct rxe_pool_entry   pelem;
};


4. IB_USER_VERBS_CMD_ALLOC_PD--->ib_uverbs_alloc_pd
{
      uobj = uobj_alloc(UVERBS_OBJECT_PD, attrs, &ib_dev);
      --->obj->type_class->alloc_begin  ---> alloc_begin_idr_uobject
      pd = rdma_zalloc_drv_obj(ib_dev, ib_pd);// rxe_pd
      ops.alloc_pd--->rxe_alloc_pd--->rxe_add_to_pool(&rxe->pd_pool, &pd->pelem);
      resp.pd_handle = uobj->id
}

struct ib_pd {
        struct ib_device       *device;
        struct ib_uobject      *uobject;
};

struct rxe_pd {
        struct ib_pd            ibpd;
        struct rxe_pool_entry   pelem;
};


5. ib_uverbs_reg_mr

pd->device->ops.reg_user_mr--->rxe_reg_user_mr


struct ib_mr {
        struct ib_device  *device;
        struct ib_pd      *pd; 
        u32                lkey;
        u32                rkey;
        u64                iova;
        u64                length;
        unsigned int       page_size;
        enum ib_mr_type    type;
        union {
                struct ib_uobject       *uobject;       /* user */
                struct list_head        qp_entry;       /* FR */
        };   
};


struct rxe_mr {
        struct rxe_pool_elem    elem;
        struct ib_mr            ibmr;

        struct ib_umem          *umem;

        u32                     lkey;
        u32                     rkey;
        enum rxe_mr_state       state;
        int                     access;
        atomic_t                num_mw;

        unsigned int            page_offset;
        unsigned int            page_shift;
        u64                     page_mask;

        u32                     num_buf;
        u32                     nbuf;

        struct xarray           page_list;
};


6. 

a. UVERBS_HANDLER(UVERBS_METHOD_CQ_CREATE) --> rxe_create_cq
--->rxe_cq_from_init ---->  rxe_queue_init
		     ---->  do_mmap_info


struct ib_ucq_object {                     
        struct ib_uevent_object uevent;
        struct list_head        comp_list;
        u32                     comp_events_reported;
};

struct ib_cq {      
        struct ib_device       *device;    
        struct ib_ucq_object   *uobject;
        ib_comp_handler         comp_handler;
        void                  (*event_handler)(struct ib_event *, void *);
        void                   *cq_context;
        int                     cqe; 
        unsigned int            cqe_used;
        enum ib_poll_context    poll_ctx;
        struct ib_wc            *wc;
        struct list_head        pool_entry;
        union {
                struct irq_poll         iop;
                struct work_struct      work;
        };                                 

        unsigned int comp_vector;
};

struct rxe_cq { 
        struct ib_cq            ibcq;
        struct rxe_pool_elem    elem;
        struct rxe_queue        *queue;
        atomic_t                num_wq;
};

b. ib_uverbs_mmap ---> rxe_mmap -->remap_vmalloc_range


7. UVERBS_HANDLER(UVERBS_METHOD_QP_CREATE)--->create_qp--->rxe_create_qp


struct ib_qp {
        struct ib_device       *device;
        struct ib_pd           *pd; 
        struct ib_cq           *send_cq;
        struct ib_cq           *recv_cq;
        int                     mrs_used;
        struct list_head        rdma_mrs;

        struct list_head        open_list;
        struct ib_qp           *real_qp;
        struct ib_uqp_object   *uobject;
        void                  (*event_handler)(struct ib_event *, void *);
        void                   *qp_context;
        /* sgid_attrs associated with the AV's */
        const struct ib_gid_attr *av_sgid_attr;
        const struct ib_gid_attr *alt_path_sgid_attr;
        u32                     qp_num;
        u32                     max_write_sge;
        u32                     max_read_sge;
        enum ib_qp_type         qp_type;
        u32                     port;
};


struct rxe_qp {
        struct ib_qp            ibqp;
        struct rxe_pool_elem    elem;
        struct ib_qp_attr       attr;
        unsigned int            valid;

        struct rxe_pd           *pd;
        struct rxe_srq          *srq;
        struct rxe_cq           *scq;
        struct rxe_cq           *rcq;

        enum ib_sig_type        sq_sig_type;

        struct rxe_sq           sq; 
        struct rxe_rq           rq; 

        struct socket           *sk;
        u32                     dst_cookie;
        u16                     src_port;

        struct sk_buff_head     req_pkts;
        struct sk_buff_head     resp_pkts;

        struct rxe_req_info     req;
        struct rxe_comp_info    comp;
        struct rxe_resp_info    resp;

        atomic_t                ssn;
        atomic_t                skb_out;
        int                     need_req_skb;
};


rxe_init_task(&qp->req.task, qp, rxe_requester);
rxe_init_task(&qp->comp.task, qp, rxe_completer);
rxe_init_task(&qp->resp.task, qp, rxe_responder);

8. ib_uverbs_modify_qp--->rxe_modify_qp

struct ib_uverbs_modify_qp {
};

qp_state_table



rmmod rdma_rxe ib_uverbs ib_core
modprobe rdma_rxe 
rdma link add rxe0 type rxe netdev eth0
./ibv_rc_pingpong -d rxe_0 -g 1
./ibv_rc_pingpong -d rxe_0 -g 1 10.0.0.7




MAX_RD_ATOMIC: set for rts
MAX_DEST_RD_ATOMIC: set to rtr
sender only support MAX_RD_ATOMIC read&atomic ops

IBV_QP_MIN_RNR_TIMER: set for rtr
IBV_QP_RNR_RETRY  :  set for rts
recever no buff send NAK_RNR to sender, sender after rnr timer(in the nack) to retry


mw/RXE_IETH_MASK
use_null_mr
IBV_WR_LOCAL_INV,
IBV_WR_BIND_MW,
IBV_WR_SEND_WITH_INV

-------------------------------------------------------------------------------------------------------------------------------------
req:

qp sig type: IB_SIGNAL_ALL_WR always ack to cqe for sender, IB_SIGNAL_REQ_WR only by req cqe flag(IB_SEND_SIGNALED)
wqe flags IB_SEND_SIGNALED: ack to cqe for sender 

IB_SEND_SOLICITED : Set the solicited event indicator for this WR. This means that when the message in this WR will be ended in the remote QP, a solicited event will be created to it and if in the remote side the user is waiting for a solicited event, it will be woken up. Relevant only for the Send and RDMA Write with immediate opcodes

IB_SEND_FENCE: Requires that all previous read and atomic operations are complete.

rd_atomic: count of unacked read and atomic operations

opcode & 0xe0: conn type(rc ud eg)
opcode & 0x1f: oper type(send write eg)

IB_OPCODE_RC_RDMA_WRITE_FIRST has the reth, No reth in MIDDLE, LAST

RC_SEND_LAST, RC_SEND_LAST_WITH_IMMEDIATE, RC_SEND_ONLY, RC_SEND_ONLY_WITH_IMMEDIATE, RC_RDMA_WRITE_LAST_WITH_IMMEDIATE, RC_RDMA_WRITE_ONLY_WITH_IMMEDIATE
RC_WRITE_LAST, RC_WRITE_ONLY, RC_RDMA_READ,  ALL_ATOMIC type will set the ack in the bth and the receiver will return a ack in rc mode

------

resp:

psn > expect psn --> out of order ----> send_ack with AETH_NAK_PSN_SEQ_ERROR syndrome, ack psn xx
psn < expect psn --> duplicate ack ----> send_ack with AETH_ACK_UNLIMITED syndrome, ack psn xx prev_psn

need new recv wqe in the receiver
IB_OPCODE_RC_SEND_FIRST, RC_SEND_ONLY, RC_SEND_ONLY_WITH_IMMEDIATE, RC_RDMA_WRITE_LAST_WITH_IMMEDIATE, RC_RDMA_WRITE_ONLY_WITH_IMMEDIATE

need delever cqe in the receiver
RC_SEND_LAST, RC_SEND_LAST_WITH_IMMEDIATE, RC_SEND_ONLY, RC_SEND_ONLY_WITH_IMMEDIATE, RC_RDMA_WRITE_LAST_WITH_IMMEDIATE, RC_RDMA_WRITE_ONLY_WITH_IMMEDIATE

delever cqe only there is recv wqe in the receiver


For WRITE_SEND if bth_ack(pkt) then send_ack

-------------------

comp:

RC_RDMA_READ_RESPONSE_MIDDLE doesn't have an AETH

Check to see if response is past the oldest WQE. if it is, complete send/write or error read/atomic : miss old ack for (send_write, but it can't be read/atomic)

do_read: IB_OPCODE_RC_RDMA_READ_RESPONSE_LAST/IB_OPCODE_RC_RDMA_READ_RESPONSE_ONLY make a cqe complete
do_atomic: RC_ATOMIC_ACKNOWLEDGE make a cqe complete
IB_OPCODE_RC_ACKNOWLEDGE FOR AETH_ACK(SEND_WRITE) make a cqe complete

comp for WRITE_SEND, the wqe->last_psn == pkt->psn then make a cqe to cq

--------------------------

/******************************************************************************
 * Base Transport Header
 ******************************************************************************/
#define BTH_SE_MASK             (0x80)
#define BTH_MIG_MASK            (0x40)
#define BTH_PAD_MASK            (0x30)

struct rxe_bth {
        u8                      opcode;
        u8                      flags;
        __be16                  pkey;
        __be32                  qpn;
        __be32                  apsn;  /*psn 0x00ffffff, ack_req 0x80000000
};


/******************************************************************************
 * Immediate Extended Transport Header
 ******************************************************************************/
struct rxe_immdt {
        __be32                  imm;
};

/******************************************************************************
 * Ack Extended Transport Header
 ******************************************************************************/
struct rxe_aeth {
        __be32                  smsn;
};

#define AETH_SYN_MASK           (0xff000000)
#define AETH_MSN_MASK           (0x00ffffff)

enum aeth_syndrome {
        AETH_TYPE_MASK          = 0xe0,
        AETH_ACK                = 0x00,
        AETH_RNR_NAK            = 0x20,
        AETH_RSVD               = 0x40,
        AETH_NAK                = 0x60,
        AETH_ACK_UNLIMITED      = 0x1f,
        AETH_NAK_PSN_SEQ_ERROR  = 0x60,
        AETH_NAK_INVALID_REQ    = 0x61,
        AETH_NAK_REM_ACC_ERR    = 0x62,
        AETH_NAK_REM_OP_ERR     = 0x63,
};

msn message(write first, middle, last as 1)  number, 



RC_RDMA_WRITE_FIRST, RC_RDMA_WRITE_ONLY, RC_RDMA_WRITE_ONLY_WITH_IMMEDIATE, IB_OPCODE_RC_RDMA_READ_REQUEST, RC_ATOMIC_WRITE

/******************************************************************************
 * RDMA Extended Transport Header
 ******************************************************************************/
struct rxe_reth {
        __be64                  va;
        __be32                  rkey;
        __be32                  len;
};


RC_COMPARE_SWAP, RC_FETCH_ADD
/******************************************************************************
 * Atomic Extended Transport Header
 ******************************************************************************/
struct rxe_atmeth {
        __be64                  va;
        __be32                  rkey;
        __be64                  swap_add;
        __be64                  comp;
} __packed;



RC_ATOMIC_ACKNOWLEDGE
/******************************************************************************
 * Atomic Ack Extended Transport Header
 ******************************************************************************/
struct rxe_atmack {
        __be64                  orig;
};




OPER:

1. Max msg size in UD is MTU
2. BTH: PAD COUNT (PADCNT) - 2 BITSPacket payloads are sent as a multiple of 4-byte quantities. Pad count in-dicates the number of pad bytes - 0 to 3 - that are appended to the packet payload. Pads are used to “stretch” the payload (payloads may be zero or more bytes in length) to be a multiple of 4 bytes.

3. The size of a SEND Operation, as generated by a requester, shall be between zero and 2^31

4. For a given requesting node’s QP, once a multi-packet SEND opera-tion is started, no other request packets may be generated until the “SEND Last”, “SEND Last with Immediate”, or “SEND Last with Inval-idate” packet is generated.

5. For an HCA requester performing RDMA WRITE operations, the length of an RDMA WRITE message, as reflected in the RETH:DMALen field, shall be between zero and 2^31

6. The RETH header is present in the first (or only) packet of the message. It contains the virtual address of the destination buffer 

7. For a given requesting node’s QP, once a multi-packet RDMA WRITE operation is started, no other request packets may be generated until the “RDMA Last” or “RDMA Last with Immediate Data” packet is sent. 

8. A single RDMA READ request can read from zero to 2-31 bytes (inclusive) of data. 

9. If the response packet BTH:Opcode is “RDMA READ Response First, RDMA READ Response Last, or RDMA READ Response Only, the packet shall also include an AETH. If the response packet BTH:Opcode is “RDMA READ Response Middle, an AETH shall not be included.

10. The virtual address in the ATOMIC Command Request packet shall be naturally aligned to an 8 byte boundary. The responding CA checks this and returns an Invalid Request NAK if it is not naturally aligned

11. If an RDMA READ work request is posted before an ATOMIC Oper-ation work request then the atomic may execute its remote memory operations before the previous RDMA READ has read its data. This can occur because the responder is allowed to delay execution of the RDMA READ. Strict ordering can be assured by posting the ATOMIC Operation work request with the fence modifier. See the description for the fence modifier Post Send Request. The fence modifier causes the requestor to wait till the RDMA READ completes before issuing the ATOMIC Operation.


transmit order :

1. A requester shall transmit request messages in the order that the Work Queue Elements (WQEs) were posted. 

2. For reliable services on an HCA, all acknowledge packets shall be strongly ordered, e.g. all previous RDMA READ responses and ATOMIC responses shall be injected into the fabric before subsequent SEND, RDMA WRITE responses, RDMA READ response or ATOMIC Operation responses.

3. A responder shall execute SEND requests, RDMA WRITE re-quests and ATOMIC Operation requests in the message order in which they are received.

4. The completion at the receiver is in the order sent (applies only to SENDs and RDMA WRITE with Immediate) and does not imply previous RDMA READs are complete unless fenced by the requester. A requester shall complete WQEs in the order in which they were transmitted. A work request with the fence attribute set shall block until all prior RDMA Read and Atomic WRs have completed. All WQEs shall be completed in the order they were posted inde-pendent of their execution order


PSN:

1. Set the AckReq bit on the last packet of every message, thus guaran-teeing that the responder will generate the needed explicit response

2. In the general PSN model, the requester calculates the PSN of the next request packet to be generated. This calculated PSN is called the Next PSN. At the time that the requester generates a new request packet, the “Next PSN” is copied into the BTH and thus becomes the current PSN. The requester then calculates a new “Next PSN”. In order to detect missing or out of order packets, the responder also cal-culates the PSN it expects to find in the next inbound request packet. This is called the Expected PSN. Conversely, when generating responses, the responder calculates the Response PSN to relate the response to a given request. However, due to acknowledge coalescing the requester cannot necessarily predict which one of a range of PSNs may appear in the next response packet. Therefore, the requester must be prepared to accept any one of a range of Response PSNs. The range is bounded by the PSN of the oldest unac-knowledged request packet and the expected response PSN of the most recently sent request packet. The requester evaluates the PSN of an in-bound response packet to ensure that it falls between these two ex-tremes. 

Requester’s Calculation of Next PSN
Current Request Packet                    PSN for Next Request Packet
SEND, RDMA WRITE, ATOMIC Operation        current PSN + 1 (modulo 2^24)
RDMA READ                                 current PSN + (number of expected RDMA READ response packets)   (modulo 2^24)


responser:

Summary: Responder Actions for Duplicate PSNs
Duplicate Request Message               Responder Action
SEND or RDMA WRITE or RESYNC          Schedule acknowledge packet
RDMA READ                           Re-execute request, schedule response
ATOMIC Operation                Do not re-execute request, after validating the request, return the saved results from the referenced ATOMIC Operation request

For an HCA responder using Reliable Connection service, for each zero-length RDMA READ or WRITE request, the R_Key shall not be validated, even if the request includes Immediate data

For an HCA, if an RDMA READ response contains more than one packet, the first and last packets must contain an AETH. 

Due to the relaxed ordering rules for RDMA READ Requests, the re-sponder is permitted to begin executing one or more SEND or RDMA WRITE requests that arrive after the RDMA READ reques

In all cases except for RDMA READ requests, the PSN of the NAK packet shall contain the responder’s expected PSN. 
In the case of an RDMA READ response packet, the PSN given in the NAK response packet shall point to the RDMA READ response packet which is being NAK’ed.   
When generating an RNR NAK, the PSN of the response packet shall point to the PSN of the packet being RNR NAK’ed. 
When generating a NAK, the packet containing the NAK code shall have an opcode of Acknowledge.

When the responder receives a valid request packet with the AckReq bit set, it shall schedule a response packet for that request. 

For SEND or RDMA WRITE requests, an ACK may be scheduled before data is actually written into the responder’s memory. The ACK simply in-dicates that the data has successfully reached the fault domain of the re-sponding node. That is, the data has been received by the channel adapter and the channel adapter will write that data to the memory system

The absence of the AckReq bit does not prohibit the responder from gen-erating a response packet. As always, RDMA READ and ATOMIC Oper-ation requests require explicit responses, thus the AckReq bit has no effect on requests.


requester:

Reliable Connected Behavior: For reliable connections (including XRC), the requester has only two possible alternatives when it receives a NAK. It may either retry the same request packet, or it may mark the cur-rent WQE as completed in error and notify its client. Note that not all NAKs can be retried.
If the requester retries the same request packet, it is not required to begin its retransmission sequence beginning with the PSN indicated in the re-sponder’s NAK; instead, it may begin its retransmission with an earlier re-quest packet. These earlier request packets are treated by the responder as normal duplicate packets causing no ill side effects.    
