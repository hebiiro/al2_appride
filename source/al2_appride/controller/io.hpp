#pragma once

namespace apn::appride::controller
{
	//
	// このクラスはコントローラの入出力部分です。
	//
	inline struct io_t
	{
		//
		// ラムダ式を使用してコンフィグを読み込みます。
		//
		inline static void read_node(const n_json& root, const std::string& node_name, auto func)
		{
			n_json node;
			read_child_node(root, node_name, node);
			return func(node);
		}

		//
		// ラムダ式を使用してコンフィグを書き込みます。
		//
		inline static void write_node(n_json& root, const std::string& node_name, auto func)
		{
			n_json node;
			my::scope_exit scope_exit([&]() { write_child_node(root, node_name, node); });
			return func(node);
		}

		//
		// プロパティを読み込みます。
		//
		void read(const n_json& root)
		{
			// 予め全てのノードを消去します。
			model::property.nodes.clear();

			// ノードを読み込みます。
			read_child_nodes(root, "node",
				[&](const n_json& root, size_t i)
			{
				// ノードを作成します。
				auto node = std::make_shared<model::property_t::node_t>();

				// ノードからノードを読み込みます。
				read_bool(root, "flag_disable", node->flag_disable);
				read_string(root, "display_name", node->display_name);
				read_string(root, "class_name", node->class_name);
				read_string(root, "window_name", node->window_name);
				read_file_name(root, "path", node->path);
				read_label(root, "exit_mode", node->exit_mode, model::property.c_exit_mode.labels);

				// ノードを追加します。
				model::property.nodes.emplace_back(node);

				return TRUE;
			});

			// その他の設定を読み込みます。
			read_bool(root, "sync_elapse", model::property.sync_elapse);
		}

		//
		// プロパティを書き込みます。
		//
		void write(n_json& root)
		{
			// ノードに書き込みます。
			write_child_nodes(root, "node", model::property.nodes,
				[&](n_json& root, const auto& node, size_t i)
			{
				// ノードにノードを書き込みます。
				write_bool(root, "flag_disable", node->flag_disable);
				write_string(root, "display_name", node->display_name);
				write_string(root, "class_name", node->class_name);
				write_string(root, "window_name", node->window_name);
				write_file_name(root, "path", node->path);
				write_label(root, "exit_mode", node->exit_mode, model::property.c_exit_mode.labels);

				return TRUE;
			});

			// その他の設定を書き込みます。
			write_bool(root, "sync_elapse", model::property.sync_elapse);
		}
	} io;
}
