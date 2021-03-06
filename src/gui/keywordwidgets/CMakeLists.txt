# Don't forget to include output directory, otherwise
# the UI file won't be wrapped!

# Meta-Objects
SET(keywordwidgets_MOC_HDRS
  atomtypeselection.h
  broadeningfunction.h
  configurationreflist.h
  dialog.h
  dropdown.h
  expressionvariablelist.h
  fileandformat.h
  isotopologueset.h
  module.h
  modulegroups.h
  modulereflist.h
  node.h
  nodeandinteger.h
  nodereflist.h
  nodevalue.h
  nodevalueenumoptions.h
  pairbroadeningfunction.h
  range.h
  speciesreflist.h
  speciessite.h
  speciessitereflist.h
  vec3double.h
  vec3integer.h
  vec3labels.h
  vec3nodevalue.h

  bool.hui
  double.hui
  dropwidget.hui
  enumoptions.hui
  integer.hui
  species.hui
  stdstring.hui
  widget.hui

  windowfunction.hui
)
QT5_WRAP_CPP(keywordwidgets_MOC_SRCS ${keywordwidgets_MOC_HDRS})

# User Interface Files
SET(keywordwidgets_UIS
  atomtypeselection.ui
  broadeningfunction.ui
  configurationreflist.ui
  dialog.ui
  dropdown.ui
  expressionvariablelist.ui
  fileandformat.ui
  isotopologueset.ui
  module.ui
  modulegroups.ui
  modulereflist.ui
  node.ui
  nodeandinteger.ui
  nodereflist.ui
  nodevalue.ui
  nodevalueenumoptions.ui
  pairbroadeningfunction.ui
  range.ui
  speciesreflist.ui
  speciessite.ui
  speciessitereflist.ui
  vec3double.ui
  vec3integer.ui
  vec3nodevalue.ui
)
QT5_WRAP_UI(keywordwidgets_UIS_H ${keywordwidgets_UIS})

# Source files (not Qt UI files or custom .h [uih])
SET(keywordwidgets_SRCS
  base.cpp

  atomtypeselection_funcs.cpp
  bool_funcs.cpp
  broadeningfunction_funcs.cpp
  configurationreflist_funcs.cpp
  dialog_funcs.cpp
  double_funcs.cpp
  dropdown_funcs.cpp
  dropwidget_funcs.cpp
  enumoptions_funcs.cpp
  expressionvariablelist_funcs.cpp
  fileandformat_funcs.cpp
  integer_funcs.cpp
  isotopologueset_funcs.cpp
  module_funcs.cpp
  modulegroups_funcs.cpp
  modulereflist_funcs.cpp
  node_funcs.cpp
  nodeandinteger_funcs.cpp
  nodereflist_funcs.cpp
  nodevalue_funcs.cpp
  nodevalueenumoptions_funcs.cpp
  pairbroadeningfunction_funcs.cpp
  range_funcs.cpp
  species_funcs.cpp
  speciesreflist_funcs.cpp
  speciessite_funcs.cpp
  speciessitereflist_funcs.cpp
  stdstring_funcs.cpp
  widget_funcs.cpp
  windowfunction_funcs.cpp
  vec3double_funcs.cpp
  vec3integer_funcs.cpp
  vec3labels.cpp
  vec3nodevalue_funcs.cpp

  base.h
)

# Target 'keywordwidgets'
add_library(keywordwidgets
  ${keywordwidgets_UIS_H}
  ${keywordwidgets_SRCS}
  ${keywordwidgets_MOC_SRCS}
)
target_include_directories(keywordwidgets PRIVATE
  ${PROJECT_SOURCE_DIR}/src
  ${PROJECT_BINARY_DIR}/src
  ${Qt5Widgets_INCLUDE_DIRS}
)
