#--------------------------------------------------------------------------
#
#  Copyright 2021 Kevin E. Jordan
#
#  Name: Makefile
#
#  Description:
#      This is a makefile for building the assembler.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#--------------------------------------------------------------------------

LIBDIR = ../_lib/
OBJDIR = _obj/

# set COS_BASE if tools are not in PATH
#COS_BASE =

CAL = $(COS_BASE)cal
LIB = $(COS_BASE)lib

LIBCOS = $(LIBDIR)libcos.lib

OBJS = \
  $(OBJDIR)args.obj  \
  $(OBJDIR)dmp.obj   \
  $(OBJDIR)iutil.obj \
  $(OBJDIR)pack.obj  \
  $(OBJDIR)sysio.obj \
  $(OBJDIR)unpack.obj

all: $(LIBCOS)

build: clean all

$(LIBDIR)libcos.lib: $(OBJS)
	-@mkdir -p $(LIBDIR)
	rm -f $(LIBCOS)
	$(LIB) -o $(LIBCOS) $(OBJS)

$(OBJDIR)%.obj: %.cal
	-@mkdir -p $(OBJDIR)
	$(CAL) -t systxt $< -o $@ -l $(@:.obj=.lst)

clean:
	rm -f $(OBJDIR)*.obj $(OBJDIR)*.lst
	rm -f $(LIBCOS)

#---------------------------  End Of File  --------------------------------
