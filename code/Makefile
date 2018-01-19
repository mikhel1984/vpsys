TARGET = vpsys
CFLAG = -Wall -g
CC = gcc
LIBS = -lm

V_BASE = main.o vp_sys.o vpb_lang.o vpb_error.o vpb_data.o vpb_memory.o \
         vpb_ticket.o vpb_element.o vpb_vars.o vpb_connectors.o \
	 vpb_array.o vpb_number.o vpb_execute.o 

V_CODE = vpc_parser.o vpc_dialog.o vpc_translate.o

V_ELEMENTS = vpe_collect1.o vpe_collect2.o vpe_block.o vpe_user.o 

OBJECTS = $(V_BASE) $(V_CODE) $(V_ELEMENTS)

.PHONY: all clean

all: $(TARGET)

clean:
	rm -rf *.o

$(TARGET): $(OBJECTS)
	${CC} ${CFLAG} $(OBJECTS) -o $(TARGET) ${LIBS}

main.o: vp_sys.h vpb_lang.h vpb_error.h
vp_sys.o: vpb_abstract.h vp_work.h vp_sys.h vp_construct.h vpb_data.h \
  vpb_memory.h vpc_parser.h vpb_error.h vpb_lang.h vpc_dialog.h \
  vpe_collect1.h vpe_collect2.h vpe_block.h
vpb_lang.o: vpb_lang.h vpb_error.h vpb_data.h vpc_parser.h
vpb_error.o: vpb_error.h vp_construct.h
vpb_data.o: vpb_data.h vpb_error.h
vpb_memory.o: vpb_memory.h vpb_ticket.h vpb_element.h vpb_error.h \
  vpb_data.h vpb_vars.h
vpc_parser.o: vpc_parser.h vpb_error.h vpb_data.h vpb_memory.h \
  vpb_element.h vpb_vars.h
vpc_dialog.o: vpc_dialog.h vp_work.h vp_construct.h vpb_memory.h \
  vpb_element.h vpb_ticket.h vpb_connectors.h vpc_parser.h \
  vpb_data.h vpb_error.h vpe_user.h vpe_block.h vpb_lang.h vpb_vars.h
vpe_collect1.o: vpe_collect1.h vp_construct.h vpb_element.h vpb_data.h \
  vpb_connectors.h vpb_error.h
vpe_collect2.o: vpe_collect2.h vp_construct.h vpb_element.h vpb_data.h \
  vpb_connectors.h vpb_error.h
vpe_block.o: vpe_block.h vpb_ticket.h vp_construct.h vp_work.h \
  vpb_element.h vpb_error.h vpb_connectors.h vpb_memory.h
vpb_ticket.o: vpb_ticket.h vpb_error.h vpb_memory.h vpb_data.h \
  vpb_element.h vp_construct.h
vpb_element.o: vpb_element.h vpb_connectors.h vpb_memory.h vpb_ticket.h \
  vpb_data.h vpb_error.h vp_construct.h vpe_block.h
vpb_vars.o: vpb_vars.h vpb_memory.h vpb_data.h vpb_element.h 
vpb_connectors.o: vpb_connectors.h vpb_data.h vpb_error.h vpb_element.h
vpe_user.o: vpe_user.h vpb_error.h vpc_parser.h vpc_translate.h \
  vpb_ticket.h vpb_connectors.h vpc_dialog.h vpb_element.h vp_construct.h
vpb_array.o: vpb_array.h vpb_number.h vpb_error.h vpb_data.h
vpb_number.o: vpb_number.h vpb_error.h vpb_data.h vpb_memory.h
vpb_execute.o: vpb_execute.h vpb_error.h vpb_element.h vpb_connectors.h \
  vp_work.h vpb_memory.h vpb_ticket.h vpe_block.h
vpc_translate.o: vpc_translate.h vpb_ticket.h vp_construct.h vpb_data.h \
  vpb_error.h vpc_parser.h vpb_element.h vpb_memory.h vpb_connectors.h
