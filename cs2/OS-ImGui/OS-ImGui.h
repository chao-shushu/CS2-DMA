#pragma once
#include "OS-ImGui_External.h"

/****************************************************
* Copyright (C)	: Liv
* @file			: OS-ImGui.h
* @author		: Liv
* @email		: 1319923129@qq.com
* @version		: 1.0
* @date			: 2023/9/17	11:25
****************************************************/

namespace OSImGui
{
#ifdef _CONSOLE
	class OSImGui : public OSImGui_External, public Singleton<OSImGui>
#else
	class OSImGui : public OSImGui_Internal, public Singleton<OSImGui>
#endif
	{
	public:
		// �ı�
		void Text(const std::string& Text, Vec2 Pos, ImColor Color, float FontSize = 15, bool KeepCenter = false);
		// ����ı�
		void StrokeText(const std::string& Text, Vec2 Pos, ImColor Color, float FontSize = 15, bool KeepCenter = false);
		// ����
		void Rectangle(Vec2 Pos, Vec2 Size, ImColor Color, float Thickness, float Rounding = 0);
		void RectangleFilled(Vec2 Pos, Vec2 Size, ImColor Color, float Rounding = 0, int Nums = 15);
		// ��
		void Line(Vec2 From, Vec2 To, ImColor Color, float Thickness);
		// Բ��
		void Circle(Vec2 Center, float Radius, ImColor Color,float Thickness, int Num = 50);
		void CircleFilled(Vec2 Center, float Radius, ImColor Color, int Num = 50);
		// ���ӵ�
		void ConnectPoints(std::vector<Vec2> Points, ImColor Color, float Thickness);
		// Բ��
		void Arc(ImVec2 Center, float Radius, ImColor Color, float Thickness, float Aangle_begin, float Angle_end, float Nums = 15);
		// ��ѡ��
		void MyCheckBox(const char* str_id, bool* v);
		void MyCheckBox2(const char* str_id, bool* v);
		void MyCheckBox3(const char* str_id, bool* v);
		void MyCheckBox4(const char* str_id, bool* v);
		// ��Ӱ����
		void ShadowRectFilled(Vec2 Pos, Vec2 Size, ImColor RectColor, ImColor ShadowColor, float ShadowThickness, Vec2 ShadowOffset, float Rounding = 0);
		// ��ӰԲ��
		void ShadowCircle(Vec2 Pos, float Radius, ImColor CircleColor, ImColor ShadowColor, float ShadowThickness, Vec2 ShadowOffset, int Num = 30);
		// Բͷ������
		bool SliderScalarEx1(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags);
	};
}

inline OSImGui::OSImGui& Gui = OSImGui::OSImGui::get();