#!/usr/bin/python

class Person:

  def __init__(self, name, age):
    self.name = name
    self.age = age
    print "initialize the %s" %self.name

  def __del__(self):
    print "%s say bye" %self.name

  def sayhi(self):
    print "%s say hi" %self.name


class Teacher(Person):

  def __init__(self, name, age, salary):
    Person.__init__(self, name, age)
    Person.salary = salary

  def sayhi(self):
    Person.sayhi(self)
    print "I am a teacher and salary is %d" %self.salary

class Student(Person):

  def __init__(self, name, age, mark):
    Person.__init__(self, name, age)
    Person.mark = mark

  def sayhi(self):
    Person.sayhi(self)
    print "I am a student and mark is %d" %self.mark


wx = Teacher("wx", 26, 13000)
wx.sayhi()


ztt = Student("ztt", 24, 100)
ztt.sayhi()

