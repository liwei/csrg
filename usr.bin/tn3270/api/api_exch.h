/*
 * This file describes the structures passed back and forth
 * between the API client and API server on a Unix-based
 * tn3270 implementation.
 *
 * A command is: <command code> <sequence number> <parameter>*
 *
 */


#define	EXCH_ASSOCIATE		23	/* Connect [client->server] */
#define	EXCH_DISASSOCIATE	39	/* Disconnect [client->server] */
#define	EXCH_SEND_AUTH		44	/* Send password [server->client] */
	/*
	 * struct storeage_desc
	 * char prompt[]
	 * struct storeage_desc
	 * char seed[]
	 */
#define	EXCH_AUTH		65	/* Authorization [client->server] */
	/*
	 * struct storeage_desc
	 * char authenticator[]
	 */
#define	EXCH_ASSOCIATED		78	/* Connected [server->client] */
#define	EXCH_REJECTED		93	/* Too bad [server->client] */
	/*
	 * struct storeage_desc
	 * char message[]
	 */

#define	EXCH_REQUEST		19	/* A request [client->server] */
	/* struct regs,
	 * struct sregs,
	 * struct storage_desc
	 * char bytes[]
	 */
#define	EXCH_GIMME		20	/* Send storage [server->client] */
	/*
	 * struct storage_desc
	 */
#define	EXCH_HEREIS		49	/* Here is storage [BOTH WAYS] */
	/*
	 * struct storage_desc
	 * char bytes[]
	 */
#define	EXCH_REPLY		87	/* End of discussion */
	/*
	 * struct regs,
	 * struct sregs,
	 */

#define	EXCH_TYPE_REGS		13
#define	EXCH_TYPE_SREGS		27
#define	EXCH_TYPE_STORE_DESC	33
#define	EXCH_TYPE_BYTES		67

/*
 * each parameter that comes over looks like:
 *
 *	char			type of following
 *	short (2 bytes)		length of following (network byte order)
 *	following
 */

struct storage_descriptor {
    long	location;	/* In network byte order */
    short	length;		/* In network byte order */
};
