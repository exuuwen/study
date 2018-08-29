#-*- coding:utf-8 -*-

import curses
from random import randrange, choice # generate and place new tile
from collections import defaultdict

letter_codes = [ord(ch) for ch in 'WSADRQwsadrq']
actions = ['Up', 'Down', 'Left', 'Right', 'Restart', 'Exit']
actions_dict = dict(zip(letter_codes, actions * 2))

def transpose(field):
    return [list(row) for row in zip(*field)]

def invert(field):
    return [row[::-1] for row in field]

class Game1k:
	def __init__(self, stdscr, width=4, height=4, win_val=2048):
		self.screen = stdscr
		self.width = width
		self.height = height
		self.win_val = win_val
		self.score = 0
		self.highscore = 0

	def reset(self):
		if self.score > self.highscore:
			self.highscore = self.score
		self.score = 0
		self.field = [[0 for i in range(self.width)] for j in range(self.height)]
		self.spawn()

	def get_user_action(self):    
		char = 'N'

		while char not in actions_dict:    
			char = self.screen.getch()
		return actions_dict[char]

	def is_win(self):
		return any(any(i >= self.win_val for i in row) for row in self.field)

	def is_gameover(self):
		return not any(self.move_is_possible(move) for move in actions)

	def spawn(self):
		new_element = 4 if randrange(100) > 89 else 2
		(i,j) = choice([(i,j) for i in range(self.width) for j in range(self.height) if self.field[i][j] == 0])
		self.field[i][j] = new_element

	def draw(self):
		help_string1 = '(W)Up (S)Down (A)Left (D)Right'
		help_string2 = '     (R)Restart (Q)Exit'
		gameover_string = '           GAME OVER'
		win_string = '          YOU WIN!'
		def cast(string):
			self.screen.addstr(string + '\n')

		def draw_hor_separator():
			line = '+' + ('+------' * self.width + '+')[1:]
			separator = defaultdict(lambda: line)
			if not hasattr(draw_hor_separator, "counter"):
				draw_hor_separator.counter = 0
			cast(separator[draw_hor_separator.counter])
			draw_hor_separator.counter += 1

		def draw_row(row):
			cast(''.join('|{: ^5} '.format(num) if num > 0 else '|      ' for num in row) + '|')

		self.screen.clear()
		cast('SCORE: ' + str(self.score))
		if 0 != self.highscore:
			cast('HIGHSCORE: ' + str(self.highscore))
		for row in self.field:
			draw_hor_separator()
			draw_row(row)
		draw_hor_separator()
		if self.is_win():
			cast(win_string)
		else:
			if self.is_gameover():
				cast(gameover_string)
			else:
				cast(help_string1)
		cast(help_string2)

	def move(self, direction):
		def move_row_left(row):
			def tighten(row): # squeese non-zero elements together
				new_row = [i for i in row if i != 0]
				new_row += [0 for i in range(len(row) - len(new_row))]
				return new_row

			def merge(row):
				pair = False
				new_row = []
				for i in range(len(row)):
					if pair:
						new_row.append(2 * row[i])
						self.score += 2 * row[i]
						pair = False
					else:
						if i + 1 < len(row) and row[i] == row[i + 1]:
							pair = True
							new_row.append(0)
						else:
							new_row.append(row[i])
				assert len(new_row) == len(row)
				return new_row
			return tighten(merge(tighten(row)))
	
		moves = {}
		moves['Left']  = lambda field:                              \
		        [move_row_left(row) for row in field]
		moves['Right'] = lambda field:                              \
		        invert(moves['Left'](invert(field)))
		moves['Up']    = lambda field:                              \
		        transpose(moves['Left'](transpose(field)))
		moves['Down']  = lambda field:                              \
                transpose(moves['Right'](transpose(field)))

		if direction in moves:
			if self.move_is_possible(direction):
				self.field = moves[direction](self.field)
				self.spawn()
				return True
		
		return False

	def move_is_possible(self, direction):
		def row_is_left_movable(row): 
			def change(i): # true if there'll be change in i-th tile
				if row[i] == 0 and row[i + 1] != 0: # Move
					return True
				if row[i] != 0 and row[i + 1] == row[i]: # Merge
					return True
				return False
			return any(change(i) for i in range(len(row) - 1))

		check = {}
		check['Left']  = lambda field:                              \
			any(row_is_left_movable(row) for row in field)

		check['Right'] = lambda field:                              \
			check['Left'](invert(field))

		check['Up']    = lambda field:                              \
			check['Left'](transpose(field))

		check['Down']  = lambda field:                              \
			check['Right'](transpose(field))

		if direction in check:
			return check[direction](self.field)
		else:
			return False


def main(stdscr):
	def init():
		game1k.reset()
		return 'Game'

	def not_game(state):
		game1k.draw()
		action = game1k.get_user_action()
		responses = defaultdict(lambda: state) #默认是当前状态，没有行为就会一直在当前界面循环
		responses['Restart'], responses['Exit'] = 'Init', 'Exit' #对应不同的行为转换到不同的状态
		return responses[action]
		
	def game():
		game1k.draw()
		action = game1k.get_user_action()

		if action == 'Restart':
			return 'Init'
		elif action == 'Exit':
			return 'Exit'
	
		if game1k.move(action):
			if game1k.is_win():
				return 'Win'
			if game1k.is_gameover():
				return 'Gameover'
	
		return 'Game'


	state_actions = {
            'Init': init,
            'Win': lambda: not_game('Win'),
            'Gameover': lambda: not_game('Gameover'),
            'Game': game
	}

	curses.use_default_colors()
	game1k = Game1k(stdscr)

	state = 'Init'

	#状态机开始循环
	while state != 'Exit':
		state = state_actions[state]()

curses.wrapper(main)
