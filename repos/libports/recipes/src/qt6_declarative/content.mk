MIRROR_FROM_REP_DIR := src/qt6/declarative/target.mk

content: $(MIRROR_FROM_REP_DIR)

$(MIRROR_FROM_REP_DIR):
	$(mirror_from_rep_dir)

PORT_DIR := $(call port_dir,$(REP_DIR)/ports/qt6_declarative)

MIRROR_FROM_PORT_DIR := src/lib/qt6_declarative

content: $(MIRROR_FROM_PORT_DIR)

$(MIRROR_FROM_PORT_DIR):
	mkdir -p $(dir $@)
	cp -r $(PORT_DIR)/$@ $(dir $@)

content: LICENSE

LICENSE:
	cp $(PORT_DIR)/src/lib/qt6_declarative/LICENSES/LGPL-3.0-only.txt $@
