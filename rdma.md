libibverbs/device.c
providers/rxe/rxe.c

1. ibv_get_device_list:

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
	/* Name of underlying kernel IB device, eg "mthca0" */
	char			name[IBV_SYSFS_NAME_MAX];
	/* Name of uverbs device, eg "uverbs0" */
	char			dev_name[IBV_SYSFS_NAME_MAX];
	/* Path to infiniband_verbs class device in sysfs */
	char			dev_path[IBV_SYSFS_PATH_MAX];
	/* Path to infiniband class device in sysfs */
	char			ibdev_path[IBV_SYSFS_PATH_MAX];
};



2. ibv_open_device  ---> 

{
   open_cdev
   rxe_alloc_context
}

struct rxe_context {
	struct verbs_context	ibv_ctx;
};

struct verbs_context {
	struct verbs_ex_private *priv;
	struct ibv_context context;	/* Must be last field in the struct */
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

3. ibv_alloc_pd ---> rxe_alloc_pd

struct ibv_pd {
        struct ibv_context     *context;
        uint32_t                handle;
};



//rxe_alloc_context


kernel:

nldev.c
rxe.c   rxe_verbs.c
uverbs_main.c   uverbs_ioctl.c
uverbs_std_types_device.c

struct rxe_ucontext {
        struct ib_ucontext ibuc;
        struct rxe_pool_entry   pelem;
};


struct rxe_dev {
        struct ib_device        ib_dev;
        struct ib_device_attr   attr;
};

struct ib_device {
        /* Do not access @dma_device directly from ULP nor from HW drivers. */
        struct device                *dma_device;
        struct ib_device_ops         ops;   ---->      rxe_dev_ops
}

uverbs_client：
struct ib_client {   
        const char *name;
	xxxxxx;
}

ib_uverbs_add_one ib_client + ib_device ---> ib_uverbs_device

struct ib_uverbs_device {
        u32                                     num_comp_vectors;
        struct ib_device        __rcu          *ib_dev;
        int                                     devnum;
        struct cdev                             cdev;
        struct uverbs_api                       *uapi;
};


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
   object_id                 method_id
UVERBS_OBJECT_DEVICE + UVERBS_METHOD_INVOKE_WRITE
   attr 
UVERBS_ATTR_WRITE_CMD ---> 
cmd_method_key |= uapi_key_write_method(def->write.command_num)















3. IB_USER_VERBS_CMD_ALLOC_PD--->ib_uverbs_alloc_pd
{
      pd = rdma_zalloc_drv_obj(ib_dev, ib_pd);
      rxe_alloc_pd
}

struct ib_pd {
        u32                     flags;
        struct ib_device       *device;
        struct ib_uobject      *uobject;
        atomic_t                usecnt; /* count all resources */
};

struct rxe_pd {
        struct ib_pd            ibpd;
        struct rxe_pool_entry   pelem;
};






























rmmod rdma_rxe ib_uverbs ib_core
./ibv_rc_pingpong -d rxe_0 -g 1
./ibv_rc_pingpong -d rxe_0 -g 1 10.0.0.7
