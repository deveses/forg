set(SOURCES
    src/EApplication.cpp src/EAsyncSocket.cpp src/EButton.cpp src/EDialog.cpp
    src/EEdit.cpp src/EImageList.cpp src/EPropertyPage.cpp src/EPropertySheet.cpp
    src/EStatusBarCtrl.cpp src/ETabCtrl.cpp src/EToolBarCtrl.cpp src/EToolWindow.cpp
    src/EWnd.cpp src/StdAfx.cpp src/emfc.cpp src/emfcs.rc)

add_library(emfc STATIC ${SOURCES})
forg_configure_target(emfc)
target_include_directories(emfc PUBLIC include PRIVATE src)
target_precompile_headers(emfc PRIVATE src/StdAfx.h)
target_compile_definitions(emfc PUBLIC EMFC_STATIC PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
target_link_libraries(emfc PUBLIC comctl32 ws2_32)
