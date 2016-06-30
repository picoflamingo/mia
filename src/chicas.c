/*
 * MIA: Mi Inteligencia Artificial (My Artificial Inteligence)
 * Copyright (c) 2015 David Mart.nez Oliveira
 *
 * This file is part of MIA
 *
 * MIA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MIA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MIA.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

#include <unistd.h>
#include <pthread.h>

#include <nyx.h>
#include <nyx_channel.h>
#include <nyx_queue.h>
#include <nyx_net.h>
#include <nyx_server.h>


#define MAX_CHICAS 4

static char   *vid[MAX_CHICAS] = {"Pepa:", "Michelle:", "Greta:", "Jane:"};
static char   *v[MAX_CHICAS] = {"es-ES", "fr-FR", "de-DE", "en-GB"};
//static int    current = -1;
//static char   *voice = "es-ES";


int
find_voice (char *name)
{
  int  i;

  for (i = 0; i < MAX_CHICAS; i++)
    {
      if (!strncasecmp (name, vid[i], strlen(vid[i])))
	return i;
    }
  return -1;
}

static int _cnt = 0;
int 
say (char *str)
{
  char               name[1024], msg[2048];
  int                the_voice, res;

  memset (msg, 0, 2048);
  strcpy (name, str);

  the_voice = find_voice (name);

  if (the_voice >= 0) 
    snprintf (msg, 2048, "(pico2wave -l %s -w /tmp/test%d.wav \"%s\"; "
	      "aplay /tmp/test%d.wav ;rm /tmp/test%d.wav) > /dev/null 2>&1", 
	      v[the_voice], _cnt, name + strlen (vid[the_voice]), _cnt, _cnt);
  else /* If voice is not recognize, use default voice and do not strip voice prefix*/
    snprintf (msg, 2048, 
	      "(pico2wave -l %s -w /tmp/test%d.wav \"%s\"; "
	      "aplay /tmp/test%d.wav ;rm /tmp/test%d.wav) > "
	      "/dev/null 2>&1", 
	      v[0], _cnt, name, 
	      _cnt, _cnt);

  
  //fprintf (stderr, "-- Running '%s'\n", msg);
  res = system (msg);

  _cnt++;

  return res;
}



void*
server_func (void *arg)
{ 
  NYX_WORKER  *w = (NYX_WORKER *) arg;
  /*
  NYX_SERVER  *s = (NYX_SERVER *) nyx_worker_get_data (w);
  NYX_QUEUE   *q = nyx_server_get_queue (s);
  */
  NYX_NET_MSG *msg;

  while (1)
    {

      msg = (NYX_NET_MSG*) nyx_queue_get (w->q);

      /* Process commands */
      say (msg->data);

      free (msg->data);
      nyx_net_msg_free (msg);
    }
}


int
main (int argc, char *argv[])
{
  NYX_NET    *net;
  NYX_SERVER *s;

  nyx_init ();
  printf ("NYX Echo Server\n");

  if (argc > 1)
    {

      if (!strncasecmp (argv[1], "-d", 2))
	{
	  fprintf (stderr, "Activating daemon mode\n");
	  if (daemon (0, 0) < 0)
	    {
	      perror ("daemon:");
	      exit (1);
	    }
	}
      else if (!strncasecmp (argv[1], "-h", 2))
	{
	  fprintf (stderr, "Usage:chicas [-d]\n-d\tDaemon Mode\n");
	  exit (1);
	}

    }


  say ("Pepa:Hola soy Pepa, la personalidad dominante del programa CHICAS.");

  s = nyx_server_new ("*", 3000);
  
  /* Create net object */
  net = nyx_net_init ();


  nyx_server_register (s, net, server_func);

  nyx_net_run (net);
  
  nyx_server_free (s);
  nyx_net_free (net);

  nyx_cleanup ();
  return 0;
}
