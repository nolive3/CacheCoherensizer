#include "MOSI_protocol.h"
#include "../sim/mreq.h"
#include "../sim/sim.h"
#include "../sim/hash_table.h"

extern Simulator *Sim;

/*************************
 * Constructor/Destructor.
 *************************/
MOSI_protocol::MOSI_protocol (Hash_table *my_table, Hash_entry *my_entry)
	: Protocol (my_table, my_entry)
{
	// Initialize lines to not have the data yet!
	this->state = MOSI_CACHE_I;
}

MOSI_protocol::~MOSI_protocol ()
{	
}

void MOSI_protocol::dump (void)
{
	const char *block_states[5] = {"X","I","S","O","M"};
	fprintf (stderr, "MOSI_protocol - state: %s\n", block_states[state]);
}

#define TYPE(rest) MOSI ## rest
//Redirect work to the appropriate function
void TYPE(_protocol)::process_cache_request (Mreq *request)
{
	switch (state) {
	case TYPE(_CACHE_I): do_cache_I (request); break;
	case TYPE(_CACHE_S): do_cache_S (request); break;
	case TYPE(_CACHE_O): do_cache_O (request); break;
	case TYPE(_CACHE_M): do_cache_M (request); break;
	case TYPE(_CACHE_IE): do_cache_IE (request); break;
	case TYPE(_CACHE_IM): do_cache_IM (request); break;
	case TYPE(_CACHE_SM): do_cache_SM (request); break;
	case TYPE(_CACHE_FM): do_cache_FM (request); break;

	default:
		fatal_error ("Invalid Cache State for TYPE Protocol\n");
	}
}
//Redirect work to the appropriate function
void TYPE(_protocol)::process_snoop_request (Mreq *request)
{
	switch (state) {
	case TYPE(_CACHE_I): do_snoop_I (request); break;
	case TYPE(_CACHE_S): do_snoop_S (request); break;
	case TYPE(_CACHE_O): do_snoop_O (request); break;
	case TYPE(_CACHE_M): do_snoop_M (request); break;
	case TYPE(_CACHE_IE): do_snoop_IE (request); break;
	case TYPE(_CACHE_IM): do_snoop_IM (request); break;
	case TYPE(_CACHE_SM): do_snoop_SM (request); break;
	case TYPE(_CACHE_FM): do_snoop_FM (request); break;

	default:
		fatal_error ("Invalid Cache State for TYPE Protocol\n");
	}
}

inline void TYPE(_protocol)::do_cache_I (Mreq *request)
{
	switch (request->msg) {
	/* 
	  No datas
	*/
	case LOAD:
		send_GETS(request->addr);
		state = TYPE(_CACHE_IE);
		/* This is a cache miss */
		Sim->cache_misses++;
		break;
	case STORE:
		send_GETM(request->addr);
		state = TYPE(_CACHE_IM);
		/* This is a cache miss */
		Sim->cache_misses++;
		break;
	default:
		request->print_msg (my_table->moduleID, "ERROR");
		fatal_error ("Client: I state shouldn't see this message\n");
	}
}

inline void TYPE(_protocol)::do_cache_S (Mreq *request)
{
	switch (request->msg) {
	/* 
	  This is like F but not 
	  allowed to send datas
	*/
	case LOAD: 
		//can haz datas, so no problemz
		send_DATA_to_proc(request->addr);
		break;
	case STORE: 
		//same as F, we can haz dataz, but have to 
		//pretend that we not can haz datas.
		send_GETM(request->addr);
		state = TYPE(_CACHE_SM);
		/* This is a cache miss */
		Sim->cache_misses++;
		break;
	default:
		request->print_msg (my_table->moduleID, "ERROR");
		fatal_error ("Client: S state shouldn't see this message\n");
	}
}

inline void TYPE(_protocol)::do_cache_O (Mreq *request)
{
	switch (request->msg) {
	/* 
	  Like F but now its dirty
	*/
	case LOAD://we can haz data, so yay!
		send_DATA_to_proc(request->addr);
		break;
	case STORE://we can haz dataz, But cant use the dataz we can haz
		send_GETM(request->addr);
		state = TYPE(_CACHE_FM);
		/* This is a cache miss */
		Sim->cache_misses++;
		break;
	default:
		request->print_msg (my_table->moduleID, "ERROR");
		fatal_error ("Client: O state shouldn't see this message\n");
	}
}

inline void TYPE(_protocol)::do_cache_M (Mreq *request)
{
	switch (request->msg) {
	/* 
	  Like E but better (Cuz its dirty)
	*/
	case LOAD:
	case STORE: // we dont have to do anything, COMPLETE FREEDOM
		send_DATA_to_proc(request->addr);
		break;
	default:
		request->print_msg (my_table->moduleID, "ERROR");
		fatal_error ("Client: M state shouldn't see this message\n");
	}
}

inline void TYPE(_protocol)::do_cache_IE (Mreq *request)
{
	switch (request->msg) {
	/* 
	  Still in I but heading to E (or S if someone sets the shared line)
	*/
	case LOAD:
	case STORE://CPU should be bloked, so this cant happen
	default:
		request->print_msg (my_table->moduleID, "ERROR");
		fatal_error ("Client: IE state shouldn't see this message\n");
	}
}

inline void TYPE(_protocol)::do_cache_IM (Mreq *request)
{
	switch (request->msg) {
	/* 
	  Still in I but heading to M
	*/
	case LOAD:
	case STORE://CPU should be bloked, so this cant happen
	default:
		request->print_msg (my_table->moduleID, "ERROR");
		fatal_error ("Client: IM state shouldn't see this message\n");
	}
}

inline void TYPE(_protocol)::do_cache_SM (Mreq *request)
{
	switch (request->msg) {
	/* 
	  Still in S but heading to M
	*/
	case LOAD:
	case STORE://CPU should be bloked, so this cant happen
	default:
		request->print_msg (my_table->moduleID, "ERROR");
		fatal_error ("Client: SM state shouldn't see this message\n");
	}
}

inline void TYPE(_protocol)::do_cache_FM (Mreq *request)
{
	switch (request->msg) {
	/* 
	  Still in F (or maybe O) but heading to M
	*/
	case LOAD:
	case STORE://CPU should be bloked, so this cant happen
	default:
		request->print_msg (my_table->moduleID, "ERROR");
		fatal_error ("Client: FM state shouldn't see this message\n");
	}
}

inline void TYPE(_protocol)::do_snoop_I (Mreq *request)
{
	switch (request->msg) {
	/*
	  See cache function 
	  with same state for 
	  description of this state
	*/
	case GETS:
	case GETM:
	case DATA://we cant haz data, so nothing to do
		break;
	default:
		request->print_msg (my_table->moduleID, "ERROR");
		fatal_error ("Client: I state shouldn't see this message\n");
	}
}

inline void TYPE(_protocol)::do_snoop_S (Mreq *request)
{
	switch (request->msg) {
	/*
	  See cache function 
	  with same state for 
	  description of this state
	*/
	case GETM:// I can haz invalidated
		state = TYPE(_CACHE_I);
	case GETS:
		// I can haz dataz, so let those 
		//jerks know they are ruining 
		//everything, and can not haz E state
		set_shared_line();
		break;
	case DATA:
	default:
		request->print_msg (my_table->moduleID, "ERROR");
		fatal_error ("Client: S state shouldn't see this message\n");
	}
}

inline void TYPE(_protocol)::do_snoop_O (Mreq *request)
{
	switch (request->msg) {
	/*
	  See cache function 
	  with same state for 
	  description of this state
	*/
	case GETM:// see F state
		state = TYPE(_CACHE_I);
	case GETS:
		set_shared_line();
		send_DATA_on_bus(request->addr,request->src_mid);
		break;
	case DATA:
	default:
		request->print_msg (my_table->moduleID, "ERROR");
		fatal_error ("Client: O state shouldn't see this message\n");
	}
}

inline void TYPE(_protocol)::do_snoop_M (Mreq *request)
{
	switch (request->msg) {
	/*
	  See cache function 
	  with same state for 
	  description of this state
	*/
	case GETM:// See E state 
		state = TYPE(_CACHE_I);
		set_shared_line();
		send_DATA_on_bus(request->addr,request->src_mid);
		break;
	case GETS:
		state = TYPE(_CACHE_O);
		set_shared_line();
		send_DATA_on_bus(request->addr,request->src_mid);
		break;
	case DATA:
	default:
		request->print_msg (my_table->moduleID, "ERROR");
		fatal_error ("Client: M state shouldn't see this message\n");
	}
}



inline void TYPE(_protocol)::do_snoop_IE (Mreq *request)
{
	switch (request->msg) {
	/*
	  See cache function 
	  with same state for 
	  description of this state
	*/
	case GETS:
	case GETM:
		break;
	case DATA://we finaly can has dataz
		// if anyone else says they haz the data then we go to S
		// otherwize they are jerks, an so we /deserve/ exclusive 
		// access
		send_DATA_to_proc(request->addr);
		state = TYPE(_CACHE_S);
		break;
	default:
		request->print_msg (my_table->moduleID, "ERROR");
		fatal_error ("Client: IE state shouldn't see this message\n");
	}
}

inline void TYPE(_protocol)::do_snoop_IM (Mreq *request)
{
	switch (request->msg) {
	/*
	  See cache function 
	  with same state for 
	  description of this state
	*/
	case GETS:
	case GETM:
		break;
	case DATA://finaly can haz datas, and now we can 
		//change it, so noone eles can haz them
		send_DATA_to_proc(request->addr);
		state = TYPE(_CACHE_M);
		break;
	default:
		request->print_msg (my_table->moduleID, "ERROR");
		fatal_error ("Client: IM state shouldn't see this message\n");
	}
}

inline void TYPE(_protocol)::do_snoop_SM (Mreq *request)
{
	switch (request->msg) {
	/*
	  See cache function 
	  with same state for 
	  description of this state
	*/
	case GETM://if we see someones GETM we invalidate our dataz,
		// so it is like we were in invalid before the CPU 
		// asked for the memories
		state = TYPE(_CACHE_IM);
	case GETS://those jerks are reading whil im trying to write 
		set_shared_line();
		break;
	case DATA://YAY! we can finaly haz dataz
		send_DATA_to_proc(request->addr);
		state = TYPE(_CACHE_M);
		break;
	default:
		request->print_msg (my_table->moduleID, "ERROR");
		fatal_error ("Client: SM state shouldn't see this message\n");
	}
}

inline void TYPE(_protocol)::do_snoop_FM (Mreq *request)
{
	switch (request->msg) {
	/*
	  See cache function 
	  with same state for 
	  description of this state
	*/
	case GETM: // if someone writes to the dataz, then invalidate
		state = TYPE(_CACHE_IM);
	case GETS://technically in F , so give the dataz to the other guy
		send_DATA_on_bus(request->addr,request->src_mid);
		set_shared_line();
		break;
	case DATA: // can haz dataz
		send_DATA_to_proc(request->addr);
		state = TYPE(_CACHE_M);
		break;
	default:
		request->print_msg (my_table->moduleID, "ERROR");
		fatal_error ("Client: FM state shouldn't see this message\n");
	}
}


