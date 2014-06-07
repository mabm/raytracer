/*
** main.c<2> for Sockets in /home/jobertomeu/Work/rush4/server/src
** 
** Made by Joris Bertomeu
** Login   <jobertomeu@epitech.net>
** 
** Started on  Sat May 17 18:43:35 2014 Joris Bertomeu
** Last update Fri May 30 12:05:47 2014 Joris Bertomeu
*/

#include "libserver.h"

void	print_error(char *str)
{
  perror("> ");
  exit(-1);
}

char	*search_ip_from_fd(t_libserver *libserver, int fd)
{
  char	*ip;
  int	i;

  i = 0;
  ip = malloc((INET_ADDRSTRLEN + 15) * sizeof(char));
  memset(ip, 0, (INET_ADDRSTRLEN + 15));
  while (i < 6)
    {
      if (libserver->clients[i].fd == fd)
	{
	  inet_ntop(AF_INET, &(libserver->clients[i].cli_addr.sin_addr.s_addr),
		    ip, INET_ADDRSTRLEN);
	}
      i++;
    }
  return (ip);
}

int	search_id_from_fd(t_libserver *libserver, int fd)
{
  int	i;

  i = 0;
  while (i < 6)
    {
      if (libserver->clients[i].fd == fd)
	return (i);
      i++;
    }
  return (-1);
}

void	send_all(t_libserver *libserver, char *addr_client, char *buff)
{
  int	i;
  char	*final;

  final = malloc(4096 * sizeof(char));
  memset(final, 0, 4096);
  i = 0;
  sprintf(final, "%s > %s", addr_client, buff);
  while (i < 6)
    {
      if (libserver->clients[i].active == 1)
	write(libserver->clients[i].fd, final, strlen(final));
      i++;
    }
  free(final);
}

void	del_client(t_libserver *libserver, char *addr_client, int fd_ok)
{
  int	id;

  id = search_id_from_fd(libserver, fd_ok);
  libserver->clients[id].active = 0;
  libserver->clients[id].idsock = 0;
  libserver->clients[id].id = 0;
  libserver->clients[id].fd = 0;
  printf("%s a quitté la conversation\n", addr_client);
}

void	parse_line(char *buff, int fd_ok, t_libserver *libserver)
{
  char	*addr_client;

  addr_client = search_ip_from_fd(libserver, fd_ok);
  if (strcmp(buff, "bye") == 0)
    del_client(libserver, addr_client, fd_ok);
  else
    printf("%s(%d) >> %s\n", addr_client, fd_ok, buff);
  send_all(libserver, addr_client, buff);
  free(addr_client);
}

void	init_clients(t_libserver *libserver)
{
  int	i;

  i = 0;
  while (i < 7)
    {
      libserver->clients[i].active = 0;
      libserver->clients[i].id = 0;
      libserver->clients[i].idsock = 0;
      libserver->clients[i].fd = 0;
      i++;
    }
}

void	init_lib(t_libserver *libserver, int port)
{
  init_clients(libserver);
  libserver->portno = port;
  libserver->id_client = 0;
  if ((libserver->sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    print_error("Socket Opening failed");
  bzero((char *) &(libserver->serv_addr), sizeof(libserver->serv_addr));
  libserver->serv_addr.sin_family = AF_INET;
  libserver->serv_addr.sin_addr.s_addr = INADDR_ANY;
  libserver->serv_addr.sin_port = htons(libserver->portno);
  if (bind(libserver->sockfd, (struct sockaddr *) &(libserver->serv_addr),
	   sizeof(libserver->serv_addr)) < 0)
    print_error("Bind error");
}

void	add_client(int	id, t_libserver *libserver)
{
  libserver->clients[libserver->id_client].cli_addr = libserver->cli_addr[0];
  libserver->clients[libserver->id_client].id = id;
  libserver->clients[libserver->id_client].active = 1;
  libserver->clients[libserver->id_client].idsock = libserver->id_client;
  libserver->clients[libserver->id_client].fd =
    libserver->fds[libserver->id_client];
}

int	id_exist(int id, t_libserver *libserver)
{
  int	i;

  i = 0;
  while (i < 6)
    {
      if (libserver->clients[i].id == id && libserver->clients[i].active == 1)
	return (1);
      i++;
    }
  return (0);
}

void	check_new_client(t_libserver *libserver)
{
  char	*addr_client;

  addr_client = malloc(INET_ADDRSTRLEN * sizeof(*addr_client));
  if (id_exist(libserver->cli_addr[0].sin_addr.s_addr, libserver) == 0)
    {
      if (libserver->id_client >= 5)
	printf("Number maximum of player reached\n");
      else
	{
	  inet_ntop(AF_INET, &(libserver->cli_addr[0].sin_addr.s_addr),
		    addr_client, INET_ADDRSTRLEN);
	  printf("-------------------\nNouveau client : %s\
\n-------------------\n\n", addr_client);
	  write(libserver->newsockfd, "Bienvenue sur le RTBertmomServer !\n",
		strlen("Bienvenue sur le RTBertmomServer !\n") + 1);
	  /* Envoi de count */
	  write(libserver->newsockfd, &count, sizeof(t_count));

	  /* Envoi de img */
	  write(libserver->newsockfd, img, sizeof(*img));

	  /* Envoi de eye */
	  write(libserver->newsockfd, &eye, sizeof(t_cam));

	  /* Envoi de items */
	  write(libserver->newsockfd, items, sizeof(t_node));

	  libserver->fds[libserver->id_client] = libserver->newsockfd;
	  add_client(libserver->cli_addr[0].sin_addr.s_addr, libserver);
	  libserver->id_client++;
	}
    }
  free(addr_client);
}

void	init_fds(t_libserver *libserver)
{
  int	i;

  i = 0;
  while (i < 6)
    libserver->fds[i++] = 0;
}

int	init_fd(fd_set	*rfds, t_libserver *libserver)
{
  int	i;
  int	max;

  i = 0;
  max = libserver->sockfd + 1;
  FD_ZERO(rfds);
  FD_SET(libserver->sockfd, rfds);
  while (i < libserver->id_client)
    {
      FD_SET(libserver->fds[i], rfds);
      max = (libserver->fds[i] >= max) ? libserver->fds[i] + 1 : max;
      i++;
    }
  return (max);
}

void	do_sock_fd(t_libserver *libserver)
{
  libserver->clilen = sizeof(libserver->cli_addr[0]);
  libserver->newsockfd = accept(libserver->sockfd,
				(struct sockaddr *)&(libserver->cli_addr[0]),
				&(libserver->clilen));
  check_new_client(libserver);
  if (libserver->newsockfd < 0)
    print_error("Acception client error");
}

void	do_sock_client(t_libserver *libserver, fd_set rfds)
{
  int	i;
  int	myint;

  i = 0;
  while (i < 6)
    {
      if (FD_ISSET(libserver->fds[i], &rfds) == 1)
	{
	  memset(libserver->buffer, 0, 4096);
	  libserver->n = read(libserver->fds[i], &myint, sizeof(int));
	  if (libserver->n <= 0)
	    print_error("Socket Read error");
	  else
	    {
	      libserver->current_id_fd = i;
	      printf("My int : %d\n", myint);
	      /* parse_line(libserver->buffer, libserver->fds[i], libserver); */
	    }
	}
      i++;
    }
}

int		server(int port)
{
  t_libserver	*libserver;
  fd_set	rfds;
  int		retval;
  int		max;

  libserver = malloc(sizeof(*libserver));
  init_lib(libserver, port);
  init_fds(libserver);
  listen(libserver->sockfd, 5);
  printf("Waiting connection on port %d ...\n", libserver->portno);
  while (1)
    {
      max = init_fd(&rfds, libserver);
      retval = select(max, &rfds, NULL, NULL, 0);
      if (retval == -1)
	printf("Error : Select\n");
      else if (retval == EINTR)
	printf("Signal Caught\n");
      else if (retval <= 6)
	if (FD_ISSET(libserver->sockfd, &rfds) == 1)
	  do_sock_fd(libserver);
	else
	  do_sock_client(libserver, rfds);
      else
	printf("Error - Max : %d retval : %d\n", max, retval);
    }
  return (0);
}

int	start_serv(int ac, char *argv[])
{
  if (ac != 2)
    {
      printf("USAGE: ./server <Port>\n");
      return (-1);
    }
  else if (atoi(argv[1]) >= 1 && atoi(argv[1]) <= 999999)
    server(atoi(argv[1]));
  else
    {
      printf("USAGE: ./server <Port>\n");
      return (-1);
    }
  return (0);
}
