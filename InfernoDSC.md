# Introduction #

This page presents a collection of recipes on how to create, manipulate and delete data structures in limbo.

Basics limbo types and module utilities included in the main distribution are included.

These materials are based on the information found in the book "Inferno Programming With Limbo" from Phillip Standley-Marbell and the inferno man pages.

# General considerations #

  1. Limbo is a strongly typed language. The compiler will not accept a program for which an unchecked type error can occur at runtime. Type checking is enforced at both compile- and runtime. Runtime type checking is necessary for situations like module loading, in which the function signatures of the module loaded from disk must match those of the module interface.
  1. Implicit type declaration is permitted and is achieved using the': =' operator. The ': =' operator permits the declaration of a variable whose type is the type of the expression being assigned to it.
  1. The basic types in Limbo —string, int, big, real and byte— are value types. They are always passed by value when passed as parameters to functions, and assignments from them yield the values they hold.

# Numbers #

## Definition ##

```
big_number: big; # -2^63 to 2^63-1
int_number: int; # -2^31 to 2^31-1
byte_number: byte; # 0 to 2^8
real_number: real; # -2^31 to 2^31-1
```

## Assignment ##
```
byte_number: byte;
int_number=7287232;
byte_number=12;
byte_number='a'+1;
```

or

```
new_int_number:=7287232;
new_real_number:= real 4/3;
```

## Reading / Access ##

```
sys->print("%d",int_number); # print a number
foo(int_number); # pass a number as a parameter to the foo function
```


# Strings #

Strings are sequences of 16-bit Unicode characters.

## Definition ##
```
s: string;
ns: string;
```

## Assignment ##

```
s="This is a string";
new_string:="this is also a string";
hello:="こんにちは";
ns[0]=112;
```

## Accesing/printing a string variable ##
```
# print a utf char
sys->print("firts letter of s is %c",s[0]);

# print utf char by utf char
fot(i:=0;i<len s;i++) {
    sys->print("letter: %c found\n",new_string[i]);
}

# printin japanese
sys->print("hello in japanese is %s",hello);

sys->print("%s",ns); # prints p character
```

## Freeing a string variable ##

```
s=nil;
```

The garbage collector will free the variable s.

## Growing a string ##

The following will store abecedary on the abc variable, growing it letter by letter:

```
abc: string;
for(i:=65;i<91;i++) {
   abc[len abc]=i;
}
sys->print("%s\n",abc);
```

Strings coung be grow utf char by utf char or concatenating two string.

## Concatenating strings ##

The operator '+' concatenate strings:

```
user:="John";
greet_message:="Hello "+user;
```

# Lists #

A list may be constructed out of elements of any of the primitive or aggregate data types, including lists.

Lists in limbo are reference types. A list variable contains a reference to a list, and not to the actual storage for items of the list. This means the operators applied to the list will operate on refereces to the objects of the list and will not modify the actual elements of the list.

## Definition ##
```
string_list: list of string;
int_list: list of int;
adt_list: list of ADT_TYPE;
name_list:=list of {"name", "surname"};
```

## Creation of lists ##

Use the operator :: to add items to the beginning of the list. You can't add items to the end of the list.

```
# add hello to the begining of the string_list
string_list="hello"::string_list;

# add 10 integers to int_list
for(i:=0;i<10;i++) {
    int_list=i::int_list;
}

```

## Lists operators ##

The 'hd' and 'tl' keywords are lists operators, 'hd' will return the first element of a list and 'tl' will return a list without the first element. 'hd' will not return a list unless you have a list of lists, and 'tl' will always return a list.

Both operators are non-destructive, that means the original list is not altered by the operators theirselfs. Of course, somethin like ` l=tl l` will alter the list l, because thats a reassgiment.

## Printing a list ##
```
l: list of string;

for(;l != nil; l = tl l) {
    e:=hd l;
    sys->print("%s",e);
}

```

## Reverse a list ##

The following function will reverse a list of any type T.

```
reverse[T](l: list of T): list of T
{
    t: list of T;
    for(; l!= nil; l=tl l)
        t=hd l::t;
    return t;
}
```

# Arrays #

Like lists, arrays are a reference type and you can make arrays of whatever datatype you have in limbo.

## Definition/Declaration ##

```
# this will declare an empty array of two elements
# but no memory will be used until some elements are added
string_array: array [2] of string;

# this will allocate the memory to store 64 ints
int_array:= array [64] of int;

# this will allocate an array of three strings
fields:= array [] of {"name","surname","phone number"};

vector: array [2] of Point;

#define an array and initialize it with a constant
zero := array [5] of {* => 0};

# define an array and initialize it with ranges
ranged_array := array [] of {
                0 or 2 or 4 or 6 or 8 => "odd",
                1 or 3 or 5 or 7 or 9 => "even"
                };

ranged_array2 := array [30] of {
                  0 to 10 => "first ten",
                  11 to 20 => "twienties",
                  * => "the rest of the array up to 30"
               };

```

## Initialization ##

```
fields: array [3] o string;
fields[0]="name";
fields[1]="surname";
fields[2]="phone";

ints: array [100] of int;
for(i:=0;i<100;i++) {
    ints[i]=i*i;
}

```

## Acess/reading ##

```
# the array ltf will contain the last two fields of the array fields
ltf:=fields[1:];

# print the second field 'surname'
sys->print("%s",field[1]);

```

## slices & arrays ##

You can't specity the end index when contatenaing two arrays, aslo one array should be less elements than the destination array, becaus it will not grow.
```
a := array [10] of int;
b := array [20] of int;

# concatenate a to b starting at 10th element
b[10:] = a;

```



---


This is still a work in progress