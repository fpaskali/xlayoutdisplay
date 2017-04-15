#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "../src/xrandrrutil.h"

using namespace std;
using ::testing::Return;

TEST(xrandrutil_renderCmd, renderAll) {
    list <DisplP> displs;

    DisplP displ1 = make_shared<Displ>("One", Displ::disconnected, list<ModeP>(), ModeP(), ModeP(), ModeP(), PosP());
    displs.push_back(displ1);

    DisplP displ2 = make_shared<Displ>("Two", Displ::disconnected, list<ModeP>(), ModeP(), ModeP(), ModeP(), PosP());
    displ2->desiredActive = true;
    displ2->desiredMode = make_shared<Mode>(1, 2, 3, 4);
    displ2->desiredPos = make_shared<Pos>(5, 6);
    displs.push_back(displ2);

    DisplP displ3 = make_shared<Displ>("Three", Displ::disconnected, list<ModeP>(), ModeP(), ModeP(), ModeP(), PosP());
    displ3->desiredActive = true;
    displ3->desiredPos = make_shared<Pos>(13, 14);
    displs.push_back(displ3);

    DisplP displ4 = make_shared<Displ>("Four", Displ::disconnected, list<ModeP>(), ModeP(), ModeP(), ModeP(), PosP());
    displ4->desiredActive = true;
    displ4->desiredMode = make_shared<Mode>(15, 16, 17, 18);
    displs.push_back(displ4);

    DisplP displ5 = make_shared<Displ>("Five", Displ::disconnected, list<ModeP>(), ModeP(), ModeP(), ModeP(), PosP());
    displ5->desiredActive = true;
    displ5->desiredMode = make_shared<Mode>(7, 8, 9, 10);
    displ5->desiredPos = make_shared<Pos>(11, 12);
    displs.push_back(displ5);

    Displ::desiredPrimary = displ2;

    const string expected = ""
            "xrandr \\\n"
            " --output One --off \\\n"
            " --output Two --mode 2x3 --rate 4 --pos 5x6 --primary \\\n"
            " --output Three --off \\\n"
            " --output Four --off \\\n"
            " --output Five --mode 8x9 --rate 10 --pos 11x12";

    EXPECT_EQ(expected, renderCmd(displs));
}

TEST(xrandrutil_renderUserInfo, renderAll) {
    ModeP mode1 = make_shared<Mode>(1, 2, 3, 4);
    ModeP mode2 = make_shared<Mode>(5, 6, 7, 8);
    ModeP mode3 = make_shared<Mode>(9, 10, 11, 12);
    PosP pos = make_shared<Pos>(13, 14);

    list <DisplP> displs;

    DisplP dis = make_shared<Displ>("dis", Displ::disconnected, list<ModeP>(), ModeP(), ModeP(), ModeP(), PosP());
    displs.push_back(dis);

    DisplP con = make_shared<Displ>("con", Displ::connected, list<ModeP>({mode1, mode2}), ModeP(), ModeP(), mode2, PosP());
    displs.push_back(con);

    DisplP act = make_shared<Displ>("act", Displ::active, list<ModeP>({mode3, mode2, mode1}), mode2, mode3, mode1, pos);
    displs.push_back(act);

    const string expected = ""
            "dis disconnected\n"
            "con connected\n"
            "   2x3 4Hz\n"
            "  !6x7 8Hz\n"
            "act active 6x7+13+14 8Hz\n"
            " + 10x11 12Hz\n"
            "*  6x7 8Hz\n"
            "  !2x3 4Hz\n"
            "*current +preferred !optimal";

    EXPECT_EQ(expected, renderUserInfo(displs));

}

class MockXRRWrapper : public XRRWrapper {
public:
    MOCK_METHOD1(xOpenDisplay, Display*(_Xconst char*));
    MOCK_METHOD1(defaultScreen, int(Display*));
    MOCK_METHOD1(screenCount, int(Display*));
    MOCK_METHOD2(rootWindow, Window(Display*, int));
    MOCK_METHOD2(xrrGetScreenResources, XRRScreenResources*(Display*, Window));
    MOCK_METHOD3(xrrGetOutputInfo, XRROutputInfo*(Display*, XRRScreenResources*, RROutput));
    MOCK_METHOD3(xrrGetCrtcInfo, XRRCrtcInfo*(Display*, XRRScreenResources*, RRCrtc));
};

class xrandrutil_discoverDispls : public ::testing::Test {
protected:
    void SetUp() override {
    }

    Display *dpy = (Display*)1;
    int screen = 2;
    Window rootWindow;
    XRRScreenResources screenResources;
};

TEST_F(xrandrutil_discoverDispls, cannotOpenDisplay) {
    MockXRRWrapper xrrWrapper;

    EXPECT_CALL(xrrWrapper, xOpenDisplay(NULL));

    EXPECT_THROW(discoverDispls(&xrrWrapper), runtime_error);
}

TEST_F(xrandrutil_discoverDispls, excessScreens) {
    MockXRRWrapper xrrWrapper;

    EXPECT_CALL(xrrWrapper, xOpenDisplay(NULL)).WillOnce(Return(dpy));
    EXPECT_CALL(xrrWrapper, defaultScreen(dpy)).WillOnce(Return(screen));
    EXPECT_CALL(xrrWrapper, screenCount(dpy)).Times(2).WillRepeatedly(Return(1));

    EXPECT_THROW(discoverDispls(&xrrWrapper), runtime_error);
}

TEST_F(xrandrutil_discoverDispls, winning) {
    MockXRRWrapper xrrWrapper;

    EXPECT_CALL(xrrWrapper, xOpenDisplay(NULL)).WillOnce(Return(dpy));
    EXPECT_CALL(xrrWrapper, defaultScreen(dpy)).WillOnce(Return(screen));
    EXPECT_CALL(xrrWrapper, screenCount(dpy)).WillOnce(Return(3));
    EXPECT_CALL(xrrWrapper, rootWindow(dpy, screen)).WillOnce(Return(rootWindow));
    EXPECT_CALL(xrrWrapper, xrrGetScreenResources(dpy, rootWindow)).WillOnce(Return(&screenResources));

    screenResources.noutput = 0;

    const list<DisplP> displs = discoverDispls(&xrrWrapper);

    EXPECT_TRUE(displs.empty());
}

// TODO: more tests!