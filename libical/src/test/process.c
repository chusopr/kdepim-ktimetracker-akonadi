/* -*- Mode: C -*-
  ======================================================================
  FILE: process.c
  CREATOR: eric 11 February 2000
  
  $Id$
  $Locker:  $
    
 (C) COPYRIGHT 2000 Eric Busboom
 http://www.softwarestudio.org

 The contents of this file are subject to the Mozilla Public License
 Version 1.0 (the "License"); you may not use this file except in
 compliance with the License. You may obtain a copy of the License at
 http://www.mozilla.org/MPL/
 
 Software distributed under the License is distributed on an "AS IS"
 basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 the License for the specific language governing rights and
 limitations under the License.
 
 ======================================================================*/

#include <stdio.h> /* for printf */
#include "ical.h"
#include "icalss.h"
#include <errno.h>
#include <string.h> /* For strerror */
#include <stdlib.h> /* for free */

struct class_map {
	ical_class class;
	char *str;
} class_map[] = {
    {ICAL_NO_CLASS,"No class"},
    {ICAL_PUBLISH_NEW_CLASS,"New Publish"},
    {ICAL_PUBLISH_UPDATE_CLASS,"New Publish"},
    {ICAL_REQUEST_NEW_CLASS,"New request"},
    {ICAL_REQUEST_UPDATE_CLASS,"Update"},
    {ICAL_REQUEST_RESCHEDULE_CLASS,"Reschedule"},
    {ICAL_REQUEST_DELEGATE_CLASS,"Delegate"},
    {ICAL_REQUEST_NEW_ORGANIZER_CLASS,"New Organizer"},
    {ICAL_REQUEST_FORWARD_CLASS,"Forward"},
    {ICAL_REQUEST_STATUS_CLASS,"Status request"},
    {ICAL_REPLY_ACCEPT_CLASS,"Accept reply"},
    {ICAL_REPLY_DECLINE_CLASS,"Decline reply"},
    {ICAL_REPLY_CRASHER_ACCEPT_CLASS,"Crasher's accept reply"},
    {ICAL_REPLY_CRASHER_DECLINE_CLASS,"Crasher's decline reply"},
    {ICAL_ADD_INSTANCE_CLASS,"Add instance"},
    {ICAL_CANCEL_EVENT_CLASS,"Cancel event"},
    {ICAL_CANCEL_INSTANCE_CLASS,"Cancel instance"},
    {ICAL_CANCEL_ALL_CLASS,"Cancel all instances"},
    {ICAL_REFRESH_CLASS,"Refresh"},
    {ICAL_COUNTER_CLASS,"Counter"},
    {ICAL_DECLINECOUNTER_CLASS,"Decline counter"},
    {ICAL_MALFORMED_CLASS,"Malformed"}, 
    {ICAL_OBSOLETE_CLASS,"Obsolete"},
    {ICAL_MISSEQUENCED_CLASS,"Missequenced"},
    {ICAL_UNKNOWN_CLASS,"Unknown"}
};

char* find_class_string(ical_class class)
{
    int i; 

    for (i = 0;class_map[i].class != ICAL_UNKNOWN_CLASS;i++){
	if (class_map[i].class == class){
	    return class_map[i].str;
	}
    }

    return "Unknown";
}

void send_message(icalcomponent *reply,const char* this_user)
{
    printf("From: %s\n\n%s\n",this_user,icalcomponent_as_ical_string(reply));
    

}


int main(int argc, char* argv[])
{
    icalcomponent *c, *next_c;
    int i=0;
    char *class_string;
    int dont_remove;
    
    icalset* f = icalset_new_file("../../test-data/process-incoming.ics");
    icalset* trash = icalset_new_file("trash.ics");
    icalset* cal = icalset_new_file("../../test-data/process-calendar.ics");
    icalset* out = icalset_new_file("outgoing.ics");

    const char* this_user = "alice@cal.softwarestudio.org";

    assert(f!= 0);
    assert(cal!=0);
    assert(trash!=0);
    assert(out!=0);


    /* Foreach incoming message */
    for(c=icalset_get_first_component(f);c!=0;c = next_c){
	
	ical_class class;
	icalcomponent *match;
	icalcomponent *inner; 
	icalcomponent *reply = 0;

	assert(c!=0);

	inner = icalcomponent_get_first_real_component(c);

	i++;
	reply = 0;
	dont_remove = 0;

	if(inner == 0){
	    printf("Bad component, no inner\n %s\n",
		   icalcomponent_as_ical_string(c));
	    continue;
	}

	/* Find a booked component that is matched to the incoming
	   message, based on the incoming component's UID, SEQUENCE
	   and RECURRENCE-ID*/

	match = icalset_fetch_match(cal,c);

	class = icalclassify(c,match,this_user);

	class_string = find_class_string(class);

	/* Print out the notes associated with the incoming component
           and the matched component in the */
	{
	    const char *c_note=0;
	    const char *m_note=0;
	    icalproperty *p;

	    for(p = icalcomponent_get_first_property(c,ICAL_X_PROPERTY);
		p!= 0;
		p = icalcomponent_get_next_property(c,ICAL_X_PROPERTY)){

		if(strcmp(icalproperty_get_x_name(p),"X-LIC-NOTE")==0){
		    c_note = icalproperty_get_x(p);
		}
	    }
	   
	    if (match != 0){
		for(p = icalcomponent_get_first_property(match,
							 ICAL_X_PROPERTY);
		    p!= 0;
		    p = icalcomponent_get_next_property(match,
							ICAL_X_PROPERTY)){
		    if(strcmp(icalproperty_get_x_name(p),"X-LIC-NOTE")==0){
			m_note = icalproperty_get_x(p);
		    }
		}
	    }
		
	    if(c_note != 0){
		printf("Incoming: %s\n",c_note);
	    }
	    if(m_note != 0){
		printf("Match   : %s\n",m_note);
	    }		
	}

	/* Main processing structure */

	switch (class){
	    case ICAL_NO_CLASS: { 
		char temp[1024];
		/* Huh? Return an error to sender */
		icalrestriction_check(c);
		icalcomponent_convert_errors(c);

		snprintf(temp,1024,"I can't understand the component you sent. \n Here is the component you sent, possibly with error messages:\n %s",icalcomponent_as_ical_string(c));

		reply = icalmessage_new_error_reply(
		    c,
		    this_user,
		    temp,
		    "",
		    ICAL_UNKNOWN_STATUS
		    );
		    
			   

		break; 
	    }
	    case ICAL_PUBLISH_NEW_CLASS: { 
		
		/* Don't accept published events from anyone but
		   self. If self, fall through to ICAL_REQUEST_NEW_CLASS */

		

	    }
	    case ICAL_REQUEST_NEW_CLASS: { 
		
		/* Book the new component if it does not overlap
		   anything. If the time is busy and the start time is
		   an even modulo 4, delegate to
		   bob@cal.softwarestudio.org. If the time is busy and
		   is 1 modulo 4, counterpropose for the first
		   available free time. Otherwise, deline the meeting */

		icalcomponent *overlaps;
		overlaps = icalclassify_find_overlaps(cal,c);

		if(overlaps == 0){
		    /* No overlaps, book the meeting */
/*		    icalset_add_component(cal,icalcomponent_new_clone(c));*/

		    /* Return a reply */
		    reply = icalmessage_new_accept_reply(c,this_user,
				  	 "I can make it to this meeting");

		    icalset_add_component(out,reply);
		  
		} else {
		    /* There was a conflict, so delegate, counterpropose
		       or decline it */
		    struct icaltimetype dtstart 
			= icalcomponent_get_dtstart(c);
		  
		    if(dtstart.hour%4 == 0){  
			/* Delegate the meeting */
			reply = icalmessage_new_delegate_reply(c, 
			  this_user,
            		  "bob@cal.softwarestudio.org",
			  "Unfortunately, I have another commitment that \
conflicts with this meeting. I am delegating my attendance to Bob. ");

			icalset_add_component(out,reply);
		      
		    } else if (dtstart.hour%4 == 1) {
			/* Counter propose to next available time */
			icalcomponent *newc;
			struct icalperiodtype next_time;

			icalspanlist *spanl = 
			    icalspanlist_new(cal,dtstart,
					     icaltime_null_time());

			next_time = icalspanlist_next_free_time(
			   spanl,icalcomponent_get_dtstart(c));

			newc = icalcomponent_new_clone(c);
			
			icalcomponent_set_dtstart(newc,next_time.start);
			

			/* Hack, the duration of the counterproposed
                           meeting may be longer than the free time
                           available */
			icalcomponent_set_duration(newc,
			     icalcomponent_get_duration(c));
			
			reply = icalmessage_new_counterpropose_reply(c, 
								     newc,
								     this_user,
			   "Unfortunately, I have another commitment that \
conflicts with this meeting. I am proposing a time that works better for me.");
		      
			icalset_add_component(out,reply);

		    } else {
			/* Decline the meeting */
		      
			reply = icalmessage_new_decline_reply(c,
							      this_user,
					  "I can't make it to this meeting");

			icalset_add_component(out,reply);

		    }

		  
		}
		break; 
	    }
	    case ICAL_PUBLISH_FREEBUSY_CLASS: {
		/* Store the busy time information in a file named after
		   the sender */
		break;
	    }
	    
	    case ICAL_PUBLISH_UPDATE_CLASS: { 
		/* Only accept publish updates from self. If self, fall
		   throught to ICAL_REQUEST_UPDATE_CLASS */
	    }

	    case ICAL_REQUEST_UPDATE_CLASS: { 
		/* always accept the changes */
		break; 
	    }
	  
	    case ICAL_REQUEST_RESCHEDULE_CLASS: { 
		/* Use same rules as REQUEST_NEW */
		icalcomponent *overlaps;
		overlaps = icalclassify_find_overlaps(cal,c);

		break; 
	    }
	    case ICAL_REQUEST_DELEGATE_CLASS: { 
	      
		break; 
	    }
	    case ICAL_REQUEST_NEW_ORGANIZER_CLASS: { 
		break; 
	    }
	    case ICAL_REQUEST_FORWARD_CLASS: { 
		break; 
	    }
	    case ICAL_REQUEST_STATUS_CLASS: { 
		break; 
	    }

	    case ICAL_REQUEST_FREEBUSY_CLASS: { 
		break; 
	    }
	    case ICAL_REPLY_ACCEPT_CLASS: { 
		/* Change the PARTSTAT of the sender */
		break; 
	    }
	    case ICAL_REPLY_DECLINE_CLASS: { 
		/* Change the PARTSTAT of the sender */
		break; 
	    }
	    case ICAL_REPLY_CRASHER_ACCEPT_CLASS: { 
		/* Add the crasher to the ATTENDEE list with the
		   appropriate PARTSTAT */
		break; 
	    }
	    case ICAL_REPLY_CRASHER_DECLINE_CLASS: { 
		/* Add the crasher to the ATTENDEE list with the
		   appropriate PARTSTAT */
		break; 
	    }
	    case ICAL_ADD_INSTANCE_CLASS: { 
		break; 
	    }
	    case ICAL_CANCEL_EVENT_CLASS: { 
		/* Remove the component */
		break; 
	    }
	    case ICAL_CANCEL_INSTANCE_CLASS: { 
		break; 
	    }
	    case ICAL_CANCEL_ALL_CLASS: { 
		/* Remove the component */	      
		break; 
	    }
	    case ICAL_REFRESH_CLASS: { 
		/* Resend the latest copy of the request */
		break; 
	    }
	    case ICAL_COUNTER_CLASS: { 
		break; 
	    }
	    case ICAL_DECLINECOUNTER_CLASS: { 
		break; 
	    }
	    case ICAL_MALFORMED_CLASS: { 
		/* Send back an error */
		break; 
	    } 
	    case ICAL_OBSOLETE_CLASS: { 
		printf(" ** Got an obsolete component:\n%s",
		       icalcomponent_as_ical_string(c));
		/* Send back an error */
		break; 
	    } 
	    case ICAL_MISSEQUENCED_CLASS: { 
		printf(" ** Got a missequenced component:\n%s",
		       icalcomponent_as_ical_string(c));
		/* Send back an error */
		break; 
	    }
	    case ICAL_UNKNOWN_CLASS: { 
		printf(" ** Don't know what to do with this component:\n%s",
		       icalcomponent_as_ical_string(c));
		/* Send back an error */
		break; 
	    }
	}

#if(0)
	if (reply != 0){	
	    
	    /* Don't send the reply if the RSVP parameter indicates not to*/
	    icalcomponent *reply_inner;
	    icalproperty *attendee;
	    icalparameter *rsvp;
	    
	    reply_inner = icalcomponent_get_first_real_component(reply);
	    attendee = icalcomponent_get_first_property(reply_inner,
							ICAL_ATTENDEE_PROPERTY);
	    rsvp = icalproperty_get_first_parameter(attendee,
						    ICAL_RSVP_PARAMETER);
	    
	    if(rsvp == 0 || icalparameter_get_rsvp(rsvp) == 1){
		icalrestriction_check(reply);
		send_message(reply,this_user);
	    }
	    
	    icalcomponent_free(reply);
	}
#endif 

	if(reply !=0){
	    printf("%s\n",icalcomponent_as_ical_string(reply));
	}
	
	next_c =   icalset_get_next_component(f);
	
	if(dont_remove == 0){
	    /*icalset_remove_component(f,c);
	      icalset_add_component(trash,c);*/
	}
    }

#if (0)
        
    for(c = icalset_get_first_component(out);
	c!=0;
	c = icalset_get_next_component(out)){
	
	printf("%s",icalcomponent_as_ical_string(c));

    }
#endif

    icalset_free(f);
    icalset_free(trash);
    icalset_free(cal);
    icalset_free(out);

    return 0;
}


