# coding=utf-8
from pwn import *
context.log_level = 'debug'

p = process('./移动客服')

def choice(num):
    p.sendlineafter('您的选择 >>' , str(num))

def add_user( name , addr , num ):
    choice(1)
    p.sendlineafter('姓名:' , name)
    p.sendlineafter('家庭地址:' , addr)
    p.sendlineafter('联系电话:' , num)
    p.sendlineafter('继续~~' , '\n')

def edit_user(index , name , addr , num ):
    choice(2)
    p.sendlineafter('编号 >>' , str(index))
    p.sendlineafter('姓名:' , name)
    p.sendlineafter('家庭地址:' , addr)
    p.sendlineafter('联系电话:' , num)
    p.sendlineafter('继续~~' , '\n')

def list( ):
    choice(3)

def add_consum( idx , num  ):
    choice(4)
    p.sendlineafter('户编号 >>' , str(idx))
    p.sendlineafter('金额:' , str(num))
    p.sendlineafter('继续~~' , '\n')

def add_recharge( idx , num  ):
    choice(5)
    p.sendlineafter('户编号 >>' , str(idx))
    p.sendlineafter('金额:' , str(num))
    p.sendlineafter('继续~~' , '\n')

add_user('Niyah' , '湖南' , "18797784377")
edit_user(3 , '田所浩二' , '下北泽' , '11451419198')

add_recharge(1 , 114514)
add_consum(2 , 1919)


p.interactive()