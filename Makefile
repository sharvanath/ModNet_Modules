MAKE  = make
DUMMY_MODULE_DIR =  dummy_module
UTILS_DIR = utils
INCLUDES = include/*.h

all: utils dummy_module 

dummy_module: $(DUMMY_MODULE_DIR)/*.c $(INCLUDES)
		$(MAKE) -C $(DUMMY_MODULE_DIR)

utils: $(UTILS_DIR)/*.c $(INCLUDES)
		$(MAKE) -C $(UTILS_DIR)

clean:
		$(MAKE) -C $(DUMMY_MODULE_DIR) clean
		$(MAKE) -C $(UTILS_DIR) clean
