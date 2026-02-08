/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: prasingh <prasingh@student.42berlin.de>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/08 11:04:02 by prasingh          #+#    #+#             */
/*   Updated: 2026/02/08 12:18:24 by prasingh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"
#include <stdio.h>
#include <stdlib.h>

int	main(int argc, char **argv)
{
	t_sim	sim;

	if (parse_and_init(&sim, argc, argv) != 0)
		return (1);
	if (run_simulation(&sim) != 0)
		return (1);
	return (sim.burnout_coder != 0);
}
