// Using the TUI takes control of the console, so we need another way to make logging messages
// visible to the user. One option is to use a TUI window that the user can open to view logs
// We can even provide functionality to allow the user to dynamically select the output verbosity!
// This is literally ripped straight out of imgui_demo.cpp. Thanks, imgui developers!

#include "imgui.h"

// XXX One change: for some reason the example object accepted a window name in the Draw() call.
// this name was passed to ImGui::Begin(), which uses it to uniquely define a window - if it's unknown,
// it will make a new window, otherwise it'll append to it.
// Multiple Draw() calls with different names would therefore instantiate multiple windows,
// but they share one buffer, which is owned by the ExampleAppLog object!
// This seems a bit odd, so move the title definition to the constructor.

// OK two small changes: second `Filter.Draw("Filter", -100.0f);` argument to -10.0f,
// this seems to affect the size of the text entry strip, which is basically zero with the default.
struct ExampleAppLog{
	ImGuiTextBuffer Buf;
	ImGuiTextFilter Filter;
	ImVector<int>   LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
	bool            AutoScroll;  // Keep scrolling if already at the bottom.
	const char* title="Log";
	
	ExampleAppLog(const char* titlein){
		AutoScroll = true;
		Clear();
		title=titlein;
	}
	
	void Clear(){
		Buf.clear();
		LineOffsets.clear();
		LineOffsets.push_back(0);
	}
	
	void AddLog(const char* fmt, ...) IM_FMTARGS(2){
		int old_size = Buf.size();
		va_list args;
		va_start(args, fmt);
		Buf.appendfv(fmt, args);
		va_end(args);
		for (int new_size = Buf.size(); old_size < new_size; old_size++){
			if (Buf[old_size] == '\n') LineOffsets.push_back(old_size + 1);
		}
	}
	
	//void Draw(const char* title, bool* p_open = NULL){ // XXX see top XXX
	void Draw(bool* p_open = NULL){  // FIXME also this implies that the passed bool can be used
		if (!ImGui::Begin(title, p_open)){ // to show or not show the window, but this line <<<
			ImGui::End();    // does not actually return false if p_open is false!
			return;          // so i don't know what this snippet is supposed to do...
		}                        // maybe something to do with collapsing the window??
		
		// Options menu
		if (ImGui::BeginPopup("Options")){
			ImGui::Checkbox("Auto-scroll", &AutoScroll);
			ImGui::EndPopup();
		}
		// Main window
		if (ImGui::Button("Options")) ImGui::OpenPopup("Options");
		ImGui::SameLine();
		bool clear = ImGui::Button("Clear");
		ImGui::SameLine();
		bool copy = ImGui::Button("Copy");
		ImGui::SameLine();
		Filter.Draw("Filter", -10.0f);
		
		ImGui::Separator();
		ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
		
		if (clear) Clear();
		if (copy){
			ImGui::LogToClipboard(); // how is this supposed to work on its own?
			//ImGui::LogText("Hello, world!"); // uncommenting these lines will copy 'hello world' to clipboard...
			//ImGui::LogFinish();
		}
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		const char* buf = Buf.begin();
		const char* buf_end = Buf.end();
		if (Filter.IsActive()){
			// In this example we don't use the clipper when Filter is enabled.
			// This is because we don't have a random access on the result on our filter.
			// A real application processing logs with ten of thousands of entries may want to store the result of
			// search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
			for (int line_no = 0; line_no < LineOffsets.Size; line_no++){
				const char* line_start = buf + LineOffsets[line_no];
				const char* line_end = 
					(line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
				if (Filter.PassFilter(line_start, line_end)){
					ImGui::TextUnformatted(line_start, line_end);
				}
			}
		} else {
			// The simplest and easy way to display the entire buffer:
			//   ImGui::TextUnformatted(buf_begin, buf_end);
			// And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
			// to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
			// within the visible area.
			// If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
			// on your side is recommended. Using ImGuiListClipper requires
			// - A) random access into your data
			// - B) items all being the  same height,
			// both of which we can handle since we an array pointing to the beginning of each line of text.
			// When using the filter (in the block of code above) we don't have random access into the data to display
			// anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
			// it possible (and would be recommended if you want to search through tens of thousands of entries).
			ImGuiListClipper clipper;
			clipper.Begin(LineOffsets.Size);
			while (clipper.Step()){
				for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++){
					const char* line_start = buf + LineOffsets[line_no];
					const char* line_end = 
						(line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
					ImGui::TextUnformatted(line_start, line_end);
				}
			}
			clipper.End();
		}
		ImGui::PopStyleVar();
		
		if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()){
			ImGui::SetScrollHereY(1.0f);
		}
		ImGui::EndChild();
		ImGui::End();
	}
};
