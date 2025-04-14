#include "SStatsWidget.h"
#include <string>
#ifdef IMGUI
#include "implot.h"
#include "SlateOptMacros.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Widgets/SViewport.h"

#define LOCTEXT_NAMESPACE "SStatsWidget"

struct ImPlotContext;

namespace Xiao
{
	static const std::string SSuccessText = TCHAR_TO_UTF8(*LOCTEXT("Success_Text", "构建成功").ToString());
	static const std::string SFailedText = TCHAR_TO_UTF8(*LOCTEXT("Failed_Text", "构建失败").ToString());
	static const std::string SCancelText = TCHAR_TO_UTF8(*LOCTEXT("Cacel_Text", "构建取消").ToString());
	static const char* SStatLabel[] = { SSuccessText.c_str(), SFailedText.c_str(), SCancelText.c_str()};
	static const std::string SDateLabel = TCHAR_TO_UTF8(*LOCTEXT("Date_Text", "日期").ToString());
	
	class FImPlot
	{
		friend class SStatsWidget;
		DECLARE_DELEGATE(FOnDraw);

		ImPlotContext* Context = nullptr;
		FVector2D CanvasSize = FVector2D::ZeroVector;

	public:
		explicit FImPlot()
		{
			Context = ImPlot::CreateContext();
		}

		~FImPlot()
		{
			if (Context)
			{
				ImPlot::DestroyContext(Context);
			}
		}

		void SetCanvasSize(const FVector2D InCanvasSize) { CanvasSize = InCanvasSize; };

		void OnBeginDraw() const
		{
			bool bOpen = true;
			ImGui::Begin("StatsPlot##", &bOpen, ImGuiWindowFlags_NoTitleBar);
			ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
			ImGui::SetWindowSize(ImVec2(CanvasSize.X, CanvasSize.Y));
		}
		
		void OnDraw()
		{
			OnBeginDraw();

			DemoHeader(LOCTEXT("PieHeaderTest_Text", "测试"), FOnDraw::CreateRaw(this, &FImPlot::StatsCharts));
			DemoHeader(LOCTEXT("PieHeaderHistory_Text", "历史记录"), FOnDraw::CreateRaw(this, &FImPlot::BuildHistory));
			DemoHeader(LOCTEXT("BuildsSection_Text", "构建历史"), FOnDraw::CreateRaw(this, &FImPlot::BuildSection));
			DemoHeader(LOCTEXT("BuildsDuration_Text", "构建时间"), FOnDraw::CreateRaw(this, &FImPlot::BuildDuration));
			
			OnEndDraw();
		}

		static void OnEndDraw()
		{
			ImGui::End();
		}

		static void DemoHeader(const FText& InLabel, const FOnDraw& InDemo)
		{
			if (ImGui::TreeNodeEx(TCHAR_TO_UTF8(*InLabel.ToString()), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow))
			{
				InDemo.Execute();
				ImGui::TreePop();
			}
		}

		void StatsCharts() 
		{
			static const std::string SBusyText = TCHAR_TO_UTF8(*LOCTEXT("Busy_Text", "繁忙").ToString());
			static const std::string SIdleText = TCHAR_TO_UTF8(*LOCTEXT("Idle_Text", "空闲").ToString());
			static const char* SOnlineMachineLabels[] = { SBusyText.c_str(), SIdleText.c_str()};
			// ImGui::SetNextItemWidth(250);

			static constexpr ImPlotAxisFlags PieFlag = ImPlotAxisFlags_NoDecorations | ImPlotAxisFlags_Lock;

			const std::string OnlineMachineText = TCHAR_TO_UTF8(*LOCTEXT("OnlineMachine_Text", "在线机器").ToString());
			if (ImPlot::BeginPlot(OnlineMachineText.c_str(), ImVec2(200,200),  ImPlotFlags_NoMouseText)) {
				ImPlot::SetupAxes(nullptr, nullptr, PieFlag, PieFlag);
				ImPlot::SetupAxesLimits(0, 1, 0, 1);
				ImPlot::PlotPieChart(SOnlineMachineLabels, OnlineStat.GetData(), 2, 0.5, 0.5, 0.4, "%.f", 90, ImPlotPieChartFlags_IgnoreHidden);
				ImPlot::EndPlot();
			}

			ImGui::SameLine();
			
			static const std::string SHelperText = TCHAR_TO_UTF8(*LOCTEXT("Helper_Text", "协助者").ToString());
			static const std::string SInitiatorText = TCHAR_TO_UTF8(*LOCTEXT("Initiator_Text", "发起者").ToString());
			static const char* BusyCoreLabel[] = { SHelperText.c_str(), SInitiatorText.c_str()};
			static const std::string BusyCoreText = TCHAR_TO_UTF8(*LOCTEXT("BusyCore_Text", "Busy核心").ToString());
			if (ImPlot::BeginPlot(BusyCoreText.c_str(), ImVec2(200,200), ImPlotFlags_NoMouseText)) {
				ImPlot::SetupAxes(nullptr, nullptr, PieFlag, PieFlag);
				ImPlot::SetupAxesLimits(0, 1, 0, 1);
				ImPlot::PlotPieChart(BusyCoreLabel, BusyCore.GetData(), 2, 0.5, 0.5, 0.4, "%.f", 90, ImPlotPieChartFlags_IgnoreHidden);
				ImPlot::EndPlot();
			}

			ImGui::SameLine();

			static const std::string BuildStatText = TCHAR_TO_UTF8(*LOCTEXT("BuildStat_Text", "构建统计").ToString());
			if (ImPlot::BeginPlot(BuildStatText.c_str(), ImVec2(200,200), ImPlotFlags_NoMouseText)) {
				ImPlot::SetupAxes(nullptr, nullptr, PieFlag, PieFlag);
				ImPlot::SetupAxesLimits(0, 1, 0, 1);
				ImPlot::PlotPieChart(SStatLabel, BuildStat.GetData(), 3, 0.5, 0.5, 0.4, "%.f", 90, ImPlotPieChartFlags_IgnoreHidden);
				ImPlot::EndPlot();
			}

			ImGui::SameLine();

			static const std::string BuildingText = TCHAR_TO_UTF8(*LOCTEXT("BuildingText_Text", "正在构建").ToString());
			if (ImPlot::BeginPlot(BuildingText.c_str(), ImVec2(200,200), ImPlotFlags_NoMouseText))
			{
				ImPlot::SetupAxes(nullptr, nullptr, PieFlag, PieFlag);
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), BuildingText.c_str());
				ImPlot::EndPlot();
			}
		}

		void BuildHistory()
		{
			// ImGui::BulletText("You can create custom plotters or extend ImPlot using implot_internal.h.");
    		double dates[]  = {1546300800,1546387200,1546473600,1546560000,1546819200,1546905600,1546992000,1547078400,1547164800,1547424000,1547510400,1547596800,1547683200,1547769600,1547942400,1548028800,1548115200,1548201600,1548288000,1548374400,1548633600,1548720000,1548806400,1548892800,1548979200,1549238400,1549324800,1549411200,1549497600,1549584000,1549843200,1549929600,1550016000,1550102400,1550188800,1550361600,1550448000,1550534400,1550620800,1550707200,1550793600,1551052800,1551139200,1551225600,1551312000,1551398400,1551657600,1551744000,1551830400,1551916800,1552003200,1552262400,1552348800,1552435200,1552521600,1552608000,1552867200,1552953600,1553040000,1553126400,1553212800,1553472000,1553558400,1553644800,1553731200,1553817600,1554076800,1554163200,1554249600,1554336000,1554422400,1554681600,1554768000,1554854400,1554940800,1555027200,1555286400,1555372800,1555459200,1555545600,1555632000,1555891200,1555977600,1556064000,1556150400,1556236800,1556496000,1556582400,1556668800,1556755200,1556841600,1557100800,1557187200,1557273600,1557360000,1557446400,1557705600,1557792000,1557878400,1557964800,1558051200,1558310400,1558396800,1558483200,1558569600,1558656000,1558828800,1558915200,1559001600,1559088000,1559174400,1559260800,1559520000,1559606400,1559692800,1559779200,1559865600,1560124800,1560211200,1560297600,1560384000,1560470400,1560729600,1560816000,1560902400,1560988800,1561075200,1561334400,1561420800,1561507200,1561593600,1561680000,1561939200,1562025600,1562112000,1562198400,1562284800,1562544000,1562630400,1562716800,1562803200,1562889600,1563148800,1563235200,1563321600,1563408000,1563494400,1563753600,1563840000,1563926400,1564012800,1564099200,1564358400,1564444800,1564531200,1564617600,1564704000,1564963200,1565049600,1565136000,1565222400,1565308800,1565568000,1565654400,1565740800,1565827200,1565913600,1566172800,1566259200,1566345600,1566432000,1566518400,1566777600,1566864000,1566950400,1567036800,1567123200,1567296000,1567382400,1567468800,1567555200,1567641600,1567728000,1567987200,1568073600,1568160000,1568246400,1568332800,1568592000,1568678400,1568764800,1568851200,1568937600,1569196800,1569283200,1569369600,1569456000,1569542400,1569801600,1569888000,1569974400,1570060800,1570147200,1570406400,1570492800,1570579200,1570665600,1570752000,1571011200,1571097600,1571184000,1571270400,1571356800,1571616000,1571702400,1571788800,1571875200,1571961600};
    		double opens[]  = {1284.7,1319.9,1318.7,1328,1317.6,1321.6,1314.3,1325,1319.3,1323.1,1324.7,1321.3,1323.5,1322,1281.3,1281.95,1311.1,1315,1314,1313.1,1331.9,1334.2,1341.3,1350.6,1349.8,1346.4,1343.4,1344.9,1335.6,1337.9,1342.5,1337,1338.6,1337,1340.4,1324.65,1324.35,1349.5,1371.3,1367.9,1351.3,1357.8,1356.1,1356,1347.6,1339.1,1320.6,1311.8,1314,1312.4,1312.3,1323.5,1319.1,1327.2,1332.1,1320.3,1323.1,1328,1330.9,1338,1333,1335.3,1345.2,1341.1,1332.5,1314,1314.4,1310.7,1314,1313.1,1315,1313.7,1320,1326.5,1329.2,1314.2,1312.3,1309.5,1297.4,1293.7,1277.9,1295.8,1295.2,1290.3,1294.2,1298,1306.4,1299.8,1302.3,1297,1289.6,1302,1300.7,1303.5,1300.5,1303.2,1306,1318.7,1315,1314.5,1304.1,1294.7,1293.7,1291.2,1290.2,1300.4,1284.2,1284.25,1301.8,1295.9,1296.2,1304.4,1323.1,1340.9,1341,1348,1351.4,1351.4,1343.5,1342.3,1349,1357.6,1357.1,1354.7,1361.4,1375.2,1403.5,1414.7,1433.2,1438,1423.6,1424.4,1418,1399.5,1435.5,1421.25,1434.1,1412.4,1409.8,1412.2,1433.4,1418.4,1429,1428.8,1420.6,1441,1460.4,1441.7,1438.4,1431,1439.3,1427.4,1431.9,1439.5,1443.7,1425.6,1457.5,1451.2,1481.1,1486.7,1512.1,1515.9,1509.2,1522.3,1513,1526.6,1533.9,1523,1506.3,1518.4,1512.4,1508.8,1545.4,1537.3,1551.8,1549.4,1536.9,1535.25,1537.95,1535.2,1556,1561.4,1525.6,1516.4,1507,1493.9,1504.9,1506.5,1513.1,1506.5,1509.7,1502,1506.8,1521.5,1529.8,1539.8,1510.9,1511.8,1501.7,1478,1485.4,1505.6,1511.6,1518.6,1498.7,1510.9,1510.8,1498.3,1492,1497.7,1484.8,1494.2,1495.6,1495.6,1487.5,1491.1,1495.1,1506.4};
    		double highs[]  = {1284.75,1320.6,1327,1330.8,1326.8,1321.6,1326,1328,1325.8,1327.1,1326,1326,1323.5,1322.1,1282.7,1282.95,1315.8,1316.3,1314,1333.2,1334.7,1341.7,1353.2,1354.6,1352.2,1346.4,1345.7,1344.9,1340.7,1344.2,1342.7,1342.1,1345.2,1342,1350,1324.95,1330.75,1369.6,1374.3,1368.4,1359.8,1359,1357,1356,1353.4,1340.6,1322.3,1314.1,1316.1,1312.9,1325.7,1323.5,1326.3,1336,1332.1,1330.1,1330.4,1334.7,1341.1,1344.2,1338.8,1348.4,1345.6,1342.8,1334.7,1322.3,1319.3,1314.7,1316.6,1316.4,1315,1325.4,1328.3,1332.2,1329.2,1316.9,1312.3,1309.5,1299.6,1296.9,1277.9,1299.5,1296.2,1298.4,1302.5,1308.7,1306.4,1305.9,1307,1297.2,1301.7,1305,1305.3,1310.2,1307,1308,1319.8,1321.7,1318.7,1316.2,1305.9,1295.8,1293.8,1293.7,1304.2,1302,1285.15,1286.85,1304,1302,1305.2,1323,1344.1,1345.2,1360.1,1355.3,1363.8,1353,1344.7,1353.6,1358,1373.6,1358.2,1369.6,1377.6,1408.9,1425.5,1435.9,1453.7,1438,1426,1439.1,1418,1435,1452.6,1426.65,1437.5,1421.5,1414.1,1433.3,1441.3,1431.4,1433.9,1432.4,1440.8,1462.3,1467,1443.5,1444,1442.9,1447,1437.6,1440.8,1445.7,1447.8,1458.2,1461.9,1481.8,1486.8,1522.7,1521.3,1521.1,1531.5,1546.1,1534.9,1537.7,1538.6,1523.6,1518.8,1518.4,1514.6,1540.3,1565,1554.5,1556.6,1559.8,1541.9,1542.9,1540.05,1558.9,1566.2,1561.9,1536.2,1523.8,1509.1,1506.2,1532.2,1516.6,1519.7,1515,1519.5,1512.1,1524.5,1534.4,1543.3,1543.3,1542.8,1519.5,1507.2,1493.5,1511.4,1525.8,1522.2,1518.8,1515.3,1518,1522.3,1508,1501.5,1503,1495.5,1501.1,1497.9,1498.7,1492.1,1499.4,1506.9,1520.9};
    		double lows[]   = {1282.85,1315,1318.7,1309.6,1317.6,1312.9,1312.4,1319.1,1319,1321,1318.1,1321.3,1319.9,1312,1280.5,1276.15,1308,1309.9,1308.5,1312.3,1329.3,1333.1,1340.2,1347,1345.9,1338,1340.8,1335,1332,1337.9,1333,1336.8,1333.2,1329.9,1340.4,1323.85,1324.05,1349,1366.3,1351.2,1349.1,1352.4,1350.7,1344.3,1338.9,1316.3,1308.4,1306.9,1309.6,1306.7,1312.3,1315.4,1319,1327.2,1317.2,1320,1323,1328,1323,1327.8,1331.7,1335.3,1336.6,1331.8,1311.4,1310,1309.5,1308,1310.6,1302.8,1306.6,1313.7,1320,1322.8,1311,1312.1,1303.6,1293.9,1293.5,1291,1277.9,1294.1,1286,1289.1,1293.5,1296.9,1298,1299.6,1292.9,1285.1,1288.5,1296.3,1297.2,1298.4,1298.6,1302,1300.3,1312,1310.8,1301.9,1292,1291.1,1286.3,1289.2,1289.9,1297.4,1283.65,1283.25,1292.9,1295.9,1290.8,1304.2,1322.7,1336.1,1341,1343.5,1345.8,1340.3,1335.1,1341.5,1347.6,1352.8,1348.2,1353.7,1356.5,1373.3,1398,1414.7,1427,1416.4,1412.7,1420.1,1396.4,1398.8,1426.6,1412.85,1400.7,1406,1399.8,1404.4,1415.5,1417.2,1421.9,1415,1413.7,1428.1,1434,1435.7,1427.5,1429.4,1423.9,1425.6,1427.5,1434.8,1422.3,1412.1,1442.5,1448.8,1468.2,1484.3,1501.6,1506.2,1498.6,1488.9,1504.5,1518.3,1513.9,1503.3,1503,1506.5,1502.1,1503,1534.8,1535.3,1541.4,1528.6,1525.6,1535.25,1528.15,1528,1542.6,1514.3,1510.7,1505.5,1492.1,1492.9,1496.8,1493.1,1503.4,1500.9,1490.7,1496.3,1505.3,1505.3,1517.9,1507.4,1507.1,1493.3,1470.5,1465,1480.5,1501.7,1501.4,1493.3,1492.1,1505.1,1495.7,1478,1487.1,1480.8,1480.6,1487,1488.3,1484.8,1484,1490.7,1490.4,1503.1};
    		double closes[] = {1283.35,1315.3,1326.1,1317.4,1321.5,1317.4,1323.5,1319.2,1321.3,1323.3,1319.7,1325.1,1323.6,1313.8,1282.05,1279.05,1314.2,1315.2,1310.8,1329.1,1334.5,1340.2,1340.5,1350,1347.1,1344.3,1344.6,1339.7,1339.4,1343.7,1337,1338.9,1340.1,1338.7,1346.8,1324.25,1329.55,1369.6,1372.5,1352.4,1357.6,1354.2,1353.4,1346,1341,1323.8,1311.9,1309.1,1312.2,1310.7,1324.3,1315.7,1322.4,1333.8,1319.4,1327.1,1325.8,1330.9,1325.8,1331.6,1336.5,1346.7,1339.2,1334.7,1313.3,1316.5,1312.4,1313.4,1313.3,1312.2,1313.7,1319.9,1326.3,1331.9,1311.3,1313.4,1309.4,1295.2,1294.7,1294.1,1277.9,1295.8,1291.2,1297.4,1297.7,1306.8,1299.4,1303.6,1302.2,1289.9,1299.2,1301.8,1303.6,1299.5,1303.2,1305.3,1319.5,1313.6,1315.1,1303.5,1293,1294.6,1290.4,1291.4,1302.7,1301,1284.15,1284.95,1294.3,1297.9,1304.1,1322.6,1339.3,1340.1,1344.9,1354,1357.4,1340.7,1342.7,1348.2,1355.1,1355.9,1354.2,1362.1,1360.1,1408.3,1411.2,1429.5,1430.1,1426.8,1423.4,1425.1,1400.8,1419.8,1432.9,1423.55,1412.1,1412.2,1412.8,1424.9,1419.3,1424.8,1426.1,1423.6,1435.9,1440.8,1439.4,1439.7,1434.5,1436.5,1427.5,1432.2,1433.3,1441.8,1437.8,1432.4,1457.5,1476.5,1484.2,1519.6,1509.5,1508.5,1517.2,1514.1,1527.8,1531.2,1523.6,1511.6,1515.7,1515.7,1508.5,1537.6,1537.2,1551.8,1549.1,1536.9,1529.4,1538.05,1535.15,1555.9,1560.4,1525.5,1515.5,1511.1,1499.2,1503.2,1507.4,1499.5,1511.5,1513.4,1515.8,1506.2,1515.1,1531.5,1540.2,1512.3,1515.2,1506.4,1472.9,1489,1507.9,1513.8,1512.9,1504.4,1503.9,1512.8,1500.9,1488.7,1497.6,1483.5,1494,1498.3,1494.1,1488.1,1487.5,1495.7,1504.7,1505.3};
    		// ImGui::SameLine();
    		static ImVec4 bullCol = ImVec4(0.000f, 1.000f, 0.441f, 1.000f);
    		static ImVec4 bearCol = ImVec4(0.853f, 0.050f, 0.310f, 1.000f);
    		// ImGui::SameLine(); ImGui::ColorEdit4("##Bull", &bullCol.x, ImGuiColorEditFlags_NoInputs);
    		// ImGui::SameLine(); ImGui::ColorEdit4("##Bear", &bearCol.x, ImGuiColorEditFlags_NoInputs);
    		ImPlot::GetStyle().UseLocalTime = false;

			static const std::string BuildHistoryText = TCHAR_TO_UTF8(*LOCTEXT("BuildHistory_Text", "构建历史").ToString());
			if (ImPlot::BeginPlot(BuildHistoryText.c_str(),ImVec2(-1,400.0f))) {
        		ImPlot::SetupAxes(nullptr,nullptr,0,ImPlotAxisFlags_AutoFit|ImPlotAxisFlags_RangeFit);
        		ImPlot::SetupAxesLimits(1546300800, 1571961600, 1250, 1600);
        		ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
        		ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, 1546300800, 1571961600);
        		ImPlot::SetupAxisZoomConstraints(ImAxis_X1, 60*60*24*14, 1571961600-1546300800);
        		ImPlot::SetupAxisFormat(ImAxis_Y1, "$%.0f");
				static bool Tooltip = true;
        		//MyImPlot::PlotCandlestick("GOOGL",dates, opens, closes, lows, highs, 218, tooltip, 0.25f, bullCol, bearCol);
        		ImPlot::EndPlot();
			}
		}

		void BuildSection()
		{
			static float RRatios[] = { 5.0f };
			static float CRatios[] = { 0.5f, 1.0f };
			static ImPlotSubplotFlags Flags = ImPlotSubplotFlags_ShareItems | ImPlotSubplotFlags_NoLegend;
			static const std::string Section1 = TCHAR_TO_UTF8(*LOCTEXT("Section1_Text", "分区1").ToString());
			if (ImPlot::BeginSubplots(Section1.c_str(), 1, 2, ImVec2(-1, 400), Flags, RRatios, CRatios)) 
			{
				DrawTotalBuildPlot();
				DrawBuildTimePlot();
				ImPlot::EndSubplots();
				
				ImGui::SameLine(5.0f);
				ImPlot::ColormapScale("##HeatScale", MinBuildDuration, MaxBuildDuration, ImVec2(60, 350), "%d m");
				ImPlot::PopColormap();
			}
		}

		void DrawTotalBuildPlot()
		{
			static const std::string TotalBuildText = TCHAR_TO_UTF8(*LOCTEXT("TotalBuild_Text", "总计构建").ToString());
			if (ImPlot::BeginPlot(TotalBuildText.c_str(), ImVec2(-1, 400)))
			{
				ImPlot::SetupLegend(ImPlotLocation_East, ImPlotLegendFlags_Outside);
				ImPlot::SetupAxisTicks(ImAxis_X1, BuildPositions.GetData(), BuildGroups, BuildDates.GetData());
				ImPlot::PlotBarGroups(SStatLabel, TotalBuild.GetData(), 3, BuildGroups, 0.5f, 0, ImPlotBarGroupsFlags_Stacked);
				ImPlot::EndPlot();
			}
		}

		void DrawBuildTimePlot()
		{
    		static const char* STimeLabel[] = {"+00","+01","+02","+03","+04","+05","+06","+07", "+08", "+09", "+10", "+11", "+12", "+13", "+14", "+15", "+16", "+17", "+18", "+19", "+20", "+21", "+22", "+23"};
    		static constexpr ImPlotAxisFlags AxesFlags = ImPlotAxisFlags_Lock | ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoTickMarks;

			static constexpr ImPlotColormap Map = ImPlotColormap_Viridis;
			static constexpr ImPlotFlags Flags = ImPlotFlags_NoLegend | ImPlotFlags_NoMenus;
			ImPlot::PushColormap(Map);

			static const std::string BuildRunTimeText = TCHAR_TO_UTF8(*LOCTEXT("BuildRunTime_Text", "构建运行时间").ToString());
    		if (ImPlot::BeginPlot(BuildRunTimeText.c_str(), ImVec2(-1, 400), Flags))
			{
    		    ImPlot::SetupAxes(nullptr, nullptr, AxesFlags, AxesFlags);
    		    ImPlot::SetupAxisTicks(ImAxis_X1, 0 + 1.0/48.0, 1 - 1.0/48.0, 24, STimeLabel);
    		    ImPlot::SetupAxisTicks(ImAxis_Y1, 1 - 1.0/14.0, 0 + 1.0/14.0, 7, BuildDates.GetData());
    		    ImPlot::PlotHeatmap("heat", DurationHeat[0].GetData(), 7, 24, MinBuildDuration, MaxBuildDuration, nullptr, ImPlotPoint(0, 0), ImPlotPoint(1, 1), ImPlotHeatmapFlags_ColMajor);
    		    ImPlot::EndPlot();
    		}
		}

		void BuildDuration()
		{
			static float RRatios[] = { 5.0f };
			static float CRatios[] = { 0.5f, 1.0f };
			static ImPlotSubplotFlags Flags = ImPlotSubplotFlags_ShareItems | ImPlotSubplotFlags_NoLegend;
			static const std::string Section2 = TCHAR_TO_UTF8(*LOCTEXT("Section2_Text", "分区2").ToString());
			if (ImPlot::BeginSubplots(Section2.c_str(), 1, 2, ImVec2(-1, 400), Flags, RRatios, CRatios))
			{
				DrawBuildPercent();
				DrawBuildDuration();
				ImPlot::EndSubplots();
			}
		}

		void DrawBuildPercent()
		{
			static const std::string BuildDurationAgentText = TCHAR_TO_UTF8(*LOCTEXT("BuildDurationAgent_Text", "构建耗时").ToString());
			if (ImPlot::BeginPlot(BuildDurationAgentText.c_str(), ImVec2()))
			{
				ImPlot::SetupLegend(ImPlotLocation_East, ImPlotLegendFlags_Outside);
				ImPlot::SetupAxisTicks(ImAxis_X1, BuildPositions.GetData(), BuildGroups, BuildDates.GetData());
				ImPlot::EndPlot();
			}
		}

		void DrawBuildDuration()
		{
			for (int i = 0; i < 7; ++i)
			{
				MeanDurations[i] = 0.45f + 0.2f * sinf(10 * i * 0.05f);
				MaxDurations [i] = 0.75f + 0.2f * sinf(10 * i * 0.05f);
			}
			static const std::string BuildDurationPlot = TCHAR_TO_UTF8(*LOCTEXT("BuildDurationPlot_Text", "构建时长").ToString());
			if (ImPlot::BeginPlot(BuildDurationPlot.c_str(), ImVec2(-1, 400)))
			{
				static const std::string YAxesText = TCHAR_TO_UTF8(*LOCTEXT("YAxes_Text", "Time[min]").ToString());
				ImPlot::SetupAxes(YAxesText.c_str(), SDateLabel.c_str());
				// ImPlot::SetupAxesLimits(0,1,0,1);

				ImPlot::PushStyleColor(ImPlotCol_Line, ImVec4(0.0f, 1.0f, 0.0f, 0.8f));
				static const std::string MeanDurationText = TCHAR_TO_UTF8(*LOCTEXT("MeanDuration_Text", "平均").ToString());
				ImPlot::PlotLine(MeanDurationText.c_str(), MeanDurations.GetData(), MeanDurations.Num(), 1.f);
				ImPlot::PopStyleColor();
				
				ImPlot::SetNextMarkerStyle(ImPlotMarker_Up);
				static const std::string MaxDurationText = TCHAR_TO_UTF8(*LOCTEXT("MaxDuration_Text", "最大").ToString());
				ImPlot::PlotScatter(MaxDurationText.c_str(), MaxDurations.GetData(), MaxDurations.Num());

				ImPlot::EndPlot();
			}
		}
	
		TArray<float> OnlineStat = {100, 20};
		TArray<float> BusyCore = {300, 21};
		TArray<float> BuildStat = {110, 20, 8};

		// 总计构建
		TArray<uint16> TotalBuild = { 83, 67, 23, 89, 83, 78, 91, 82, 85, 90,  // midterm
									  80, 62, 56, 99, 55, 78, 88, 78, 90, 100, // final
									  80, 69, 52, 92, 72, 78, 75, 76, 89, 95 }; // course
		TArray<double> BuildPositions = { 0,1,2,3,4,5,6,7,8,9 };
		int BuildGroups = BuildPositions.Num();

		float MinBuildDuration = 0.0f;
		float MaxBuildDuration = 6.3f;

		TArray<float> MeanDurations = { 0.8f, 2.4f, 2.5f, 3.9f, 0.0f, 4.0f, 0.0f};
		TArray<float> MaxDurations = { 0.8f, 2.4f, 2.5f, 3.9f, 0.0f, 4.0f, 0.0f};

		// 热力图
		TArray<TArray<float>> DurationHeat = { {0.8f, 2.4f, 2.5f, 3.9f, 0.0f, 4.0f, 0.0f, 0.8f, 0.8f, 0.9f},
									{2.4f, 0.0f, 4.0f, 1.0f, 2.7f, 0.0f, 0.0f},
									{1.1f, 2.4f, 0.8f, 4.3f, 1.9f, 4.4f, 0.0f},
									{0.6f, 0.0f, 0.3f, 0.0f, 3.1f, 0.0f, 0.0f},
									{0.7f, 1.7f, 0.6f, 2.6f, 2.2f, 6.2f, 0.0f},
									{1.3f, 1.2f, 0.0f, 0.0f, 0.0f, 3.2f, 5.1f},
									{0.1f, 2.0f, 0.0f, 1.4f, 0.0f, 1.9f, 6.3f} };

		// 当前查看日期
		TArray<const char*> BuildDates = { "17", "18", "19", "20", "21", "22", "23", "24", "25", "26"};

		// 代理名称
		TArray<const char*> AgentNames = {"Ghost", "Hash", "Shen", "Chen"};
		TArray<const char*> Percents = {"0.25", "0.25", "0.25", "0.25"};
	};	
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SStatsWidget::Construct(const FArguments& InArgs)
{
	check(InArgs._Viewport.IsValid());

	Viewport = InArgs._Viewport;
	
	SImGuiBase::Construct(SImGuiBase::FArguments()
		.Viewport(InArgs._Viewport)
		// .ModuleManager(InArgs._ModuleManager)
		.ContextIndex(-1)
	);

	Plot = MakeShared<Xiao::FImPlot>();
}

FVector2D SStatsWidget::ComputeDesiredSize(float) const
{
	FVector2D Size;
	if(Viewport.IsValid())
	{
		Size = Viewport.Pin()->ComputeDesiredSize(1.0f);
	}
	const auto AbsPos = this->GetCachedGeometry().AbsolutePosition;
	const float DPIScaleFactor = FPlatformApplicationMisc::GetDPIScaleFactorAtPoint(AbsPos.X, AbsPos.Y);
	FVector2D CanvasSize = Size / DPIScaleFactor;
	CanvasSize.X -= 218;
	CanvasSize.Y -= 185;
	Plot->SetCanvasSize(CanvasSize);
	return Size;
}

void SStatsWidget::OnDraw()
{
	bool bDebugPlot = true;
	// ImPlot::ShowDemoWindow(&bDebugPlot);
	Plot->OnDraw();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#endif

#undef LOCTEXT_NAMESPACE