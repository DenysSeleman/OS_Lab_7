#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <signal.h>
#include <sys/types.h>
using namespace std;

int f(int x)
{
	if (x % 3 == 0)
		return 1;
	else if (x % 3 == 1)
		return 0;
	else
		while (1);
}

int g(int x)
{
	if (x == 0 || x == 1)
		return 1;
	else if (x == 2 || x == 4)
		return 0;
	else
		while (1);
}

int main()
{
	int x;
	cout << "Enter x: ";
	cin >> x;

	int child1_to_parent[2];
	int child2_to_parent[2];
	int parent_to_child1[2];
	int parent_to_child2[2];
	int res1 = -1, res2 = -1, val;
	int status1 = 0, status2 = 0;

	pipe(child1_to_parent);
	pipe(child2_to_parent);
	pipe(parent_to_child1);
	pipe(parent_to_child2);

	pid_t firstChild, secondChild;

	if ((firstChild = fork()) < 0)
	{
		cerr << "Can not fork the process! (1)";
		exit(EXIT_FAILURE);
	}

	if (firstChild == 0)
	{
		read(parent_to_child1[0], &val, sizeof(val));
		res1 = f(val);
		write(child1_to_parent[1], &res1, sizeof(res1));
		exit(EXIT_SUCCESS);
	}

	if ((secondChild = fork()) < 0)
	{
		cerr << "Can not fork the process! (2)";
		exit(EXIT_FAILURE);
	}

	if (secondChild == 0)
	{
		read(parent_to_child2[0], &val, sizeof(val));
		res2 = g(val);
		write(child2_to_parent[1], &res2, sizeof(res2));
		exit(EXIT_SUCCESS);
	}

	write(parent_to_child1[1], &x, sizeof(x));
	write(parent_to_child2[1], &x, sizeof(x));

	int time = 1;
	pid_t firstComplete = 0;
	pid_t secondComplete = 0;
	bool keepAsking = true;
	bool firstKilled = false;
	bool secondKilled = false;

	while (true)
	{
		firstComplete = waitpid(firstChild, &status1, WNOHANG);
		secondComplete = waitpid(secondChild, &status2, WNOHANG);

		if (firstComplete && !firstKilled)
		{
			read(child1_to_parent[0], &res1, sizeof(res1));
			kill(firstChild, SIGKILL);
			firstKilled = true;
		}

		if (secondComplete && !secondKilled)
		{
			read(child2_to_parent[0], &res2, sizeof(res2));
			kill(secondChild, SIGKILL);
			secondKilled = true;
		}

		if (res1 == 0 || res2 == 0 || (firstComplete && secondComplete))
		{
			if (!firstKilled)
			{
				kill(firstChild, SIGKILL);
				firstKilled = true;
			}
			if (!secondKilled)
			{
				kill(secondChild, SIGKILL);
				secondKilled = true;
			}
			break;
		}

		if (keepAsking && time % 5 == 0)
		{
			char answer = '0';
			cout << "Do you want to continue calculations? Enter 1 to continue, 2 to stop, 3 to continue without asking again. ";

			while (answer != '1' && answer != '2' && answer != '3')
			{
				cin >> answer;
				if (answer != '1' && answer != '2' && answer != '3')
					cout << "Input Error! Please try again. ";
			}

			if (answer == '2')
			{
				if (!firstKilled)
				{
					kill(firstChild, SIGKILL);
					firstKilled = true;
				}
				if (!secondKilled)
				{
					kill(secondChild, SIGKILL);
					secondKilled = true;
				}
				break;
			}
			else if (answer == '3')
				keepAsking = false;
		}

		sleep(1);
		time++;
	}

	if (res1 == 0 || res2 == 0 || (firstComplete && secondComplete))
		cout << "f(x) && g(x) = " << (res1 && res2) << endl;
	else
		cout << "Value of (f(x) && g(x)) was not calculated." << endl;

    return 0;
}
