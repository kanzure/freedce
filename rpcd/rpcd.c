/*
 * 
 * (c) Copyright 1989 OPEN SOFTWARE FOUNDATION, INC.
 * (c) Copyright 1989 HEWLETT-PACKARD COMPANY
 * (c) Copyright 1989 DIGITAL EQUIPMENT CORPORATION
 * To anyone who acknowledges that this file is provided "AS IS"
 * without any express or implied warranty:
 *                 permission to use, copy, modify, and distribute this
 * file for any purpose is hereby granted without fee, provided that
 * the above copyright notices and this notice appears in all source
 * code copies, and that none of the names of Open Software
 * Foundation, Inc., Hewlett-Packard Company, or Digital Equipment
 * Corporation be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  Neither Open Software Foundation, Inc., Hewlett-
 * Packard Company, nor Digital Equipment Corporation makes any
 * representations about the suitability of this software for any
 * purpose.
 * 
 */
/*
 */
/*
**
**  NAME:
**
**      rpcd.c
**
**  FACILITY:
**
**      RPC Daemon
**
**  ABSTRACT:
**
**  This daemon is a catch all for DCE RPC support functions.  This server
**  exports the DCE 1.0 endpoint map (ept_) network interfaces and, optionally,  
**  the NCS 1.5.1 llb_ network interfaces and .  Additionally, this server 
**  provides the RPC forwarding map function used by non-connection oriented
**  protocol services.
**
**
*/

#include <commonp.h>
#include <com.h>

#include <dce/ep.h>
EXTERNAL ept_v3_0_epv_t ept_v3_0_mgr_epv;

#ifdef RPC_LLB
#  include <dce/llb.h>
EXTERNAL llb__v4_0_epv_t llb_v4_0_mgr_epv;
#endif

#include <comfwd.h>

#include <dsm.h>

#include <rpcdp.h>
#include <rpcddb.h>
#include <rpcdepdb.h>

#ifdef RPC_LLB
#include <rpcdlbdb.h>
#endif
          
#include <dce/dce_error.h>


#if defined(UNIX) || defined(unix)
#include <sys/ioctl.h>
#include <sys/stat.h>
#endif
#include <locale.h>

INTERNAL void usage
    _DCE_PROTOTYPE_ ((
        void
    ));

#if 0
INTERNAL boolean32 match_command
    _DCE_PROTOTYPE_ ((
        char           *key,
        char           *str,
        long           min_len
    ));
#endif

INTERNAL void process_args
    _DCE_PROTOTYPE_ ((
        int             argc,
        char            *argv[]
    ));

INTERNAL void register_ifs
    _DCE_PROTOTYPE_ ((
        error_status_t  *status
    ));

INTERNAL void use_protseqs
    _DCE_PROTOTYPE_ ((
        error_status_t  *status
    ));

INTERNAL void init
    _DCE_PROTOTYPE_ ((
        error_status_t  *status
    ));


INTERNAL void fwd_map
    _DCE_PROTOTYPE_ ((
        uuid_p_t                object,
        rpc_if_id_p_t           interface,
        rpc_syntax_id_p_t       data_rep,
        rpc_protocol_id_t       rpc_protocol,
        unsigned32              rpc_protocol_vers_major,
        unsigned32              rpc_protocol_vers_minor,
        rpc_addr_p_t            addr,
        uuid_p_t                actuuid,
        rpc_addr_p_t            *fwd_addr,
        rpc_fwd_action_t        *fwd_action,
        error_status_t          *status
    ));



/*
 * These implementation constants can be redefined in the system specific 
 * config files if necessary (e.g. common/ultrix_mips.h).
 */

#ifdef DCELOCAL_PATH
#  define rpcd_c_database_name_prefix1 DCELOCAL_PATH
#  define rpcd_c_database_name_prefix2 "/var/rpc/" 
#else

#ifndef rpcd_c_database_name_prefix1 
#  define rpcd_c_database_name_prefix1 "/tmp/"
#endif

#ifndef rpcd_c_database_name_prefix2
#  define rpcd_c_database_name_prefix2 ""
#endif

#endif

#ifndef rpcd_c_ep_database_name
#  define rpcd_c_ep_database_name "rpcdep.dat"
#endif

#ifndef rpcd_c_llb_database_name
#   define rpcd_c_llb_database_name "rpcdllb.dat"
#endif

#ifndef rpcd_c_logfile_name
#   define rpcd_c_logfile_name "rpcd.log"
#endif


/* 
 *  Optional list of protocol sequences which rpcd will use.
 *  List is specified on the command line
 */
#define                 MAX_PROTSEQ_ARGS            8
INTERNAL unsigned_char_p_t protseq[MAX_PROTSEQ_ARGS];
INTERNAL unsigned32     num_protseq = 0;
INTERNAL boolean32      use_all_protseqs = true;

/*
 *  Debug flag controls
 */
GLOBAL   boolean32      dflag = false;
#define  DEBUG_LEVEL    "0.1"

INTERNAL boolean32      foreground = false;

INTERNAL char           rpcd_version_str[] = "rpcd version dce 1.1";

GLOBAL   uuid_t         nil_uuid;



PRIVATE boolean32 check_st_bad(str, st)
char            *str;
error_status_t  *st;
{
    if (STATUS_OK(st)) 
        return false;

    show_st(str, st);
    return true;
}

PRIVATE void show_st(str, st)
char            *str;
error_status_t  *st;
{
    dce_error_string_t estr;
    int             tmp_st;

    dce_error_inq_text(*st, estr, &tmp_st);
    fprintf(stderr, "(rpcd) %s: (0x%lx) %s\n", str, *st, estr);
}

INTERNAL void usage()
{
#ifdef DEBUG
    fprintf(stderr, "usage: rpcd [-vDuf] [-d<debug switches>] [<protseq> ...]\n");
#else
    fprintf(stderr, "usage: rpcd [-vuf] [<protseq> ...]\n");
#endif
    fprintf(stderr, "  -v: Print rpcd version and exit\n");
#ifdef DEBUG
    fprintf(stderr, "  -D: Turns on default RPC runtime debug output\n");
#endif
    fprintf(stderr, "  -u: Print this message and exit\n");
    fprintf(stderr, "  -f: Run in foreground (default is to fork and parent exit)\n");
#ifdef DEBUG
    fprintf(stderr, "  -d: Turns on specified RPC runtime debug output\n");
#endif
    fprintf(stderr, "  If any <protseq>s are specified, the rpcd listens only on those; otherwise\n");
    fprintf(stderr, "  all protseqs are listened on.\n");
}

/* 
 * match_command 
 * takes a key and string as input and returns whether the
 * string matches the key where at least min_len characters
 * of the key are required to be specified.
 */
#if 0
INTERNAL boolean32 match_command(key,str,min_len)
char *key;
char *str;
long min_len;
{
    int i = 0;

    if (*key) while (*key == *str) {
        i++;
        key++;
        str++;
        if (*str == '\0' || *key == '\0')
            break;
    }
    if (*str == '\0' && i >= min_len)
        return true;
    return false;
}
#endif


/*
 *  Process args
 */
INTERNAL void process_args(argc, argv)
int             argc;
char            *argv[];
{
    int             i, c;
    unsigned32      status;
    extern int      optind;
    extern char     *optarg;
    

    /*
     * Process args.
     */

    while ((c = getopt(argc, argv, "vufDd:")) != EOF)
    {
        switch (c)
        {
        case 'u':
            usage();
            exit(0);

        case 'v':
            printf("\t%s\n", rpcd_version_str);
            exit(0);

        case 'f':
            foreground = true;
            break;

        case 'd':
        case 'D':
            rpc__dbg_set_switches(c == 'd' ? optarg : DEBUG_LEVEL, &status);
            if (check_st_bad("Error setting debug switches", &status))
                return;
            dflag = true;
            break;

        default:
            usage();
            exit(1);
        }
    }

    argc -= optind - 1;
    argv = &argv[optind - 1];

    use_all_protseqs = (argc == 1);

    for (i = 1; i < argc; i++)
    {

        rpc_network_is_protseq_valid((unsigned_char_p_t)argv[i], &status);
        if (check_st_bad("Protseq is not valid", &status))
            return;

        if (num_protseq >= MAX_PROTSEQ_ARGS)
        {
            SET_STATUS(&status, ept_s_cant_perform_op); 
            show_st("Too many protseq args", &status);
            return;
        }

        protseq[num_protseq++] = (unsigned_char_p_t) argv[i];
    }
}

/*
 *  Register the Endpoint Map and LLB interfaces.
 */
INTERNAL void register_ifs(status)
error_status_t  *status;
{
    rpc_server_register_if(ept_v3_0_s_ifspec, (uuid_p_t)NULL, 
            (rpc_mgr_epv_t) &ept_v3_0_mgr_epv, status);
    if (check_st_bad("Unable to rpc_server_register_if for ept", status))
        return;

#ifdef RPC_LLB
    rpc_server_register_if(llb__v4_0_s_ifspec, (uuid_p_t)NULL, 
            (rpc_mgr_epv_t) &llb_v4_0_mgr_epv, status);
    if (check_st_bad("Unable to rpc_server_register_if for llb", status))
        return;
#endif
}

/*
 * Arrange to handle calls on the protocol sequences of interest.
 * Note that while both interfaces specify well know endpoints,
 * the ept_ endpoints are a superset of the llb_ endpoints (which 
 * precludes us from doing a "use_protseq_if" on both).
 */
INTERNAL void use_protseqs(status)
error_status_t  *status;
{
    unsigned32 i;

    if (use_all_protseqs)
    {
        rpc_server_use_all_protseqs_if(0, ept_v3_0_s_ifspec, status);
        if (! STATUS_OK(status)) 
        {
            if (*status == rpc_s_cant_bind_sock)
                show_st("Verify that no other rpcd/llbd is running", status);
            else
                show_st("Unable to rpc_server_use_all_protseqs for ept", status);
        }
    }
    else
    {
        for (i = 0; i < num_protseq; i++)
        {
            rpc_server_use_protseq_if(protseq[i], 0, ept_v3_0_s_ifspec, status);
            if (! STATUS_OK(status)) 
            {
                if (*status == rpc_s_cant_bind_sock)
                    show_st("Verify that no other rpcd/llbd is running", status);
                else
                    show_st("Unable to rpc_server_use_all_protseqs for ept", status);
            }
        }
    }


    /*
     * If folks are interested, tell em what we're listening on...
     */

    if (dflag)
    {
        rpc_binding_vector_p_t  bv;
        unsigned_char_p_t       bstr;
        error_status_t          st;

        printf("(rpcd) got bindings:\n");

        rpc_server_inq_bindings(&bv, status);
        if (check_st_bad("Unable to rpc_server_inq_bindings", status))
            return;

        for (i = 0; i < bv->count; i++)
        {   
            rpc_binding_to_string_binding(bv->binding_h[i], &bstr, &st);
            printf("    %s\n", bstr);
            rpc_string_free(&bstr, &st);
        }

        rpc_binding_vector_free(&bv, status);
        if (check_st_bad("Unable to rpc_binding_vector_free", status))
            return;
    }
}

/*
 * Do some server database, ... initialization
 */
INTERNAL void init(status)
error_status_t  *status;
{
    epdb_handle_t       h;
    uuid_t              epdb_obj;
    rpc_if_rep_p_t      ept_if_rep;
    unsigned_char_p_t   fname;

    uuid_create_nil(&nil_uuid, status);
    if (check_st_bad("Can't create nil uuid", status))
        return;

    if (dflag)
        printf("(rpcd) initializing database\n"); 

    fname = (unsigned_char_p_t) malloc(strlen(rpcd_c_database_name_prefix1) + 
                                       strlen(rpcd_c_database_name_prefix2) + 
                                       strlen(rpcd_c_ep_database_name) + 1);
    sprintf((char *) fname, "%s%s%s", rpcd_c_database_name_prefix1,
            rpcd_c_database_name_prefix2, rpcd_c_ep_database_name);

    h = epdb_init(fname, status);
    if (check_st_bad("Can't initialize ept database", status))
        return;

    free(fname);
    
#ifdef RPC_LLB
    fname = (unsigned_char_p_t) malloc(strlen(rpcd_c_database_name_prefix1) + 
                                       strlen(rpcd_c_database_name_prefix2) + 
                                       strlen(rpcd_c_llb_database_name) + 1);
    sprintf((char *) fname, "%s%s%s", rpcd_c_database_name_prefix1,
            rpcd_c_database_name_prefix2, rpcd_c_llb_database_name);

    lbdb_init(fname, status);
    if (check_st_bad("Can't initialize llb database", status))
        return;

    free(fname);
#endif

    epdb_inq_object(h, &epdb_obj, status);
    if (check_st_bad("Can't get ept object uuid", status))	{
		 /* do nothing */;
	 }
    ept_if_rep = (rpc_if_rep_p_t) ept_v3_0_s_ifspec;
    rpc_object_set_type(&epdb_obj, &ept_if_rep->id, status);
    if (check_st_bad("Can't set ept object type", status))	{
		 /* do nothing */;
	 }

    if (dflag)
    {
        unsigned_char_p_t   ustr;
        error_status_t      st;

        uuid_to_string(&epdb_obj, &ustr, &st);
        printf("(rpcd) endpoint database object id: %s\n", ustr);
    }
}


/*
 * Perform the forwarding map algorithm to produce an rpc_addr to the
 * selected endpoint.
 *
 * Eventually, we probably want to get all packets from a single activity
 * to a single server (assuming that we can figure out how take advantage
 * of selecting different potential servers in the face of stale entries).
 * 
 */
INTERNAL void fwd_map
    (object, interface, data_rep, 
    rpc_protocol, rpc_protocol_vers_major, rpc_protocol_vers_minor, addr, 
    actuuid, fwd_addr, fwd_action, status)
uuid_p_t                object;
rpc_if_id_p_t           interface;
rpc_syntax_id_p_t       data_rep;
rpc_protocol_id_t       rpc_protocol;
unsigned32              rpc_protocol_vers_major;
unsigned32              rpc_protocol_vers_minor;
rpc_addr_p_t            addr;
uuid_p_t                actuuid __attribute__((__unused__));
rpc_addr_p_t            *fwd_addr;
rpc_fwd_action_t        *fwd_action;
error_status_t          *status;
{
    unsigned32      num_ents;
    epdb_handle_t   h;

    /*
     * Forwarding algorithm:
     * Consult ep database (and possibly the llb database) to see if 
     * anybody has registered the matching interface/object uuids.  
     */

    num_ents = 0;

    h = epdb_inq_handle();
    epdb_fwd(h, object, interface, data_rep, 
             rpc_protocol, rpc_protocol_vers_major, rpc_protocol_vers_minor, 
             addr, NULL, 1L, &num_ents, fwd_addr, status);

#ifdef RPC_LLB
    if ((*status == ept_s_not_registered) ||
        ((*status == rpc_s_ok) && (num_ents == 0)) )
    {
        h = lbdb_inq_handle();
        lbdb_fwd(h, object, &interface->uuid, addr, 
                        NULL, 1L, &num_ents, fwd_addr, status);
    }
#endif

    if (*status != rpc_s_ok)
    {
        if (*status == ept_s_not_registered)
        {
            *fwd_action = rpc_e_fwd_drop;
            *status = rpc_s_ok;
        }
        return;
    }

    assert(num_ents <= 1);

    *fwd_action = num_ents == 0 ? rpc_e_fwd_drop : rpc_e_fwd_forward;
    return;
}

int main(int argc, char *argv[])
{
    error_status_t  status;
    int uid ;

    /* begin */

    setlocale(LC_ALL, "");

    process_args(argc, argv);

#if defined(UNIX) || defined(unix)
    /*
     * Must be root (pid=0) to be able to start llbd
     */
    uid = getuid();
    if (uid != 0) {  
        fprintf(stderr, "(rpcd) Must be root to start rpcd, your uid = %d \n", uid);
        exit(2);
    }
#endif


    /*
     * If not debugging, fork off a process to be the rpcd.  The parent exits.
     */

    if (!dflag && !foreground) 
    {
#if defined(UNIX) || defined(unix)
        int pid, fd;

#ifdef TIOCNOTTY
        /*
         * Lose our controlling TTY, fork off the child to be the real rpcd,
         * and make the child a process group leader.
         */

        if ((fd = open("/dev/tty", O_RDWR)) >= 0)
        {
            ioctl(fd, TIOCNOTTY, 0);
            close(fd);
        }
#endif

        pid = fork();
        if (pid > 0)
            exit(0);

#if defined(__linux__)
	setpgrp();
#else
        setpgrp(0, getpid());
#endif
        {
            char *fname;
            char *p;

            if ((fname = malloc(strlen(rpcd_c_database_name_prefix1) + 
                           strlen(rpcd_c_database_name_prefix2) + 
                           strlen(rpcd_c_logfile_name) + 1)) != NULL)
            {
                sprintf((char *) fname, "%s%s%s", rpcd_c_database_name_prefix1,
                        rpcd_c_database_name_prefix2, rpcd_c_logfile_name);
                if ((fd = open(fname, O_WRONLY|O_CREAT|O_TRUNC,
                               S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) != -1)
                {
                    (void)dup2(fd,2);
                }
                (void)close(fd);
                /*
                 * We don't care if open() or dup2() failed.
                 */

                if ((p = strrchr(fname, (int)'/')) != NULL)
                {
                    *p = '\0';
                    (void)chdir(fname);
                    /*
                     * Again, we don't care if chdir() failed.
                     */
                }
                free(fname);
            }
        }
#endif
    }

    /*
     * Initialize the runtime by calling this public routine
     */
    rpc_network_is_protseq_valid ((unsigned_char_p_t) "ncadg_ip_udp", &status);

    /*
     * Initialize the database and other misc stuff.
     */
    init(&status);
    if (! STATUS_OK(&status)) exit(1);

    register_ifs(&status);
    if (! STATUS_OK(&status)) exit(1);

    use_protseqs(&status);
    if (! STATUS_OK(&status)) exit(1);

    /*
     * Start handling RPCs and performing forwarding.
     */

    if (dflag)
        printf("(rpcd) listening...\n");

    rpc__server_register_fwd_map(fwd_map, &status);
    if (check_st_bad("Unable to rpc_server_register_fwd_map", &status))
        exit(1);

    rpc_server_listen(5, &status);
    if (check_st_bad("Unable to rpc_server_listen", &status))
        exit(1);
    
    exit(0);
}
