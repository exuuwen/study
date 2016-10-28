#!/usr/bin/python

class Person:
  population = 0

  def __init__(self, name, age):
    self.name = name#public
    self.__age = age#private
    print "initialize the %s" %self.name
    Person.population += 1
    print "population %d" %Person.population
    
  def __del__(self):
    print "%s say bye" %self.name
#    Person.population -= 1
#    print "population %d" %Person.population

  def howold(self):
    print "%s is %d old" %(self.name, self.__age)

  def sayhi(self):
    print "%s say hi" %self.name

  def howmany(self):
    return Person.population


wen = Person("wen xu", 26)
wen.sayhi()
wen.howold()
print "%s number %d" %(wen.name, wen.howmany())

# can not "zou.__age"
zou = Person("zou tt", 24)
zou.sayhi()
zou.howold()
print "%s number %d" %(wen.name, Person.population)#%zou.population


