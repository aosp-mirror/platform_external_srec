###############################################################################
# FILE: Makefile
#
# Top-level Makefile for UAPI
###############################################################################

# remember root of UAPI source tree
export UAPI_ROOT_DIR := $(call my-dir)
export UAPI_MAKE_DIR := $(UAPI_ROOT_DIR)/make/uapi

# perform sub-makes
include $(call all-subdir-makefiles)
