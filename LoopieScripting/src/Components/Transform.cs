using System;

namespace Loopie
{
	public class Transform : Component
	{
		public enum Space
		{
			LocalSpace,
			WorldSpace
		}

		public Vector3 position
		{
			get { return GetPosition(); }
			set { SetPosition(value); }
		}

		private Vector3 GetPosition()
		{
			Vector3 entityPosition = Vector3.Zero;
			InternalCalls.Transform_GetPosition(entity.ID, out entityPosition);
			return entityPosition;
		}
		private void SetPosition(Vector3 position)
		{
			InternalCalls.Transform_SetPosition(entity.ID, ref position);
		}

		public Vector3 local_position
		{
			get { return GetLocalPosition(); }
			set { SetLocalPosition(value); }
		}

		private Vector3 GetLocalPosition()
		{
			Vector3 entityPosition = Vector3.Zero;
			InternalCalls.Transform_GetLocalPosition(entity.ID, out entityPosition);
			return entityPosition;
		}
		private void SetLocalPosition(Vector3 position)
		{
			InternalCalls.Transform_SetLocalPosition(entity.ID, ref position);
		}

		public Vector3 rotation
		{
			get { return GetRotation(); }
			set { SetRotation(value); }
		}

		private Vector3 GetRotation()
		{
			Vector3 entityPosition = Vector3.Zero;
			InternalCalls.Transform_GetRotation(entity.ID, out entityPosition);
			return entityPosition;
		}
		private void SetRotation(Vector3 position)
		{
			InternalCalls.Transform_SetRotation(entity.ID, ref position);
		}

		public Vector3 local_rotation
		{
			get { return GetLocalRotation(); }
			set { SetLocalRotation(value); }
		}

		private Vector3 GetLocalRotation()
		{
			Vector3 entityPosition = Vector3.Zero;
			InternalCalls.Transform_GetLocalRotation(entity.ID, out entityPosition);
			return entityPosition;
		}
		private void SetLocalRotation(Vector3 position)
		{
			InternalCalls.Transform_SetLocalRotation(entity.ID, ref position);
		}

		public Vector3 scale
		{
			get { return GetLocalScale(); }
			set { SetLocalScale(value); }
		}

		private Vector3 GetLocalScale()
		{
			Vector3 entityPosition = Vector3.Zero;
			InternalCalls.Transform_GetLocalScale(entity.ID, out entityPosition);
			return entityPosition;
		}
		private void SetLocalScale(Vector3 position)
		{
			InternalCalls.Transform_SetLocalScale(entity.ID, ref position);
		}

		public void Translate(Vector3 translation, Space space)
		{
			InternalCalls.Transform_Translate(entity.ID, ref translation, space);
		}
		public void Rotate(Vector3 eulerAngles, Space space)
		{
			InternalCalls.Transform_Rotate(entity.ID, ref eulerAngles, space);
		}
		public void LookAt(Vector3 target, Vector3 up_vector)
		{
			InternalCalls.Transform_LookAt(entity.ID, ref target, ref up_vector);
		}

		public Vector3 Forward
		{ get { return GetForward(); } }

		public Vector3 Back
		{ get { return GetBack(); } }

		public Vector3 Up
		{ get { return GetUp(); } }

		public Vector3 Down
		{ get { return GetDown(); } }

		public Vector3 Left
		{ get { return GetLeft(); } }

		public Vector3 Right
		{ get { return GetRight(); } }

		private Vector3 GetForward()
		{
			Vector3 vector = Vector3.Zero;
			InternalCalls.Transform_Forward(entity.ID, out vector);
			return vector;
		}
		private Vector3 GetBack()
		{
			Vector3 vector = Vector3.Zero;
			InternalCalls.Transform_Back(entity.ID, out vector);
			return vector;
		}
		private Vector3 GetUp()
		{
			Vector3 vector = Vector3.Zero;
			InternalCalls.Transform_Up(entity.ID, out vector);
			return vector;
		}
		private Vector3 GetDown()
		{
			Vector3 vector = Vector3.Zero;
			InternalCalls.Transform_Down(entity.ID, out vector);
			return vector;
		}
		private Vector3 GetLeft()
		{
			Vector3 vector = Vector3.Zero;
			InternalCalls.Transform_Left(entity.ID, out vector);
			return vector;
		}
		private Vector3 GetRight()
		{
			Vector3 vector = Vector3.Zero;
			InternalCalls.Transform_Right(entity.ID, out vector);
			return vector;
		}
	}

	public class RectTransform : Transform
	{
		public Vector2 size
		{
			get { return GetSize(); }
			set { SetSize(value); }
		}
		private Vector2 GetSize()
		{
			Vector2 size = Vector2.Zero;
			InternalCalls.RectTransform_GetSize(entity.ID, out size);
			return size;
		}
		private void SetSize(Vector2 size)
		{
			InternalCalls.RectTransform_SetSize(entity.ID, ref size);
		}

		public Vector2 pivot
		{
			get { return GetPivot(); }
			set { SetPivot(value); }
		}

		private Vector2 GetPivot()
		{
			Vector2 pivot = Vector2.Zero;
			InternalCalls.RectTransform_GetPivot(entity.ID, out pivot);
			return pivot;
		}
		private void SetPivot(Vector2 pivot)
		{
			InternalCalls.RectTransform_SetPivot(entity.ID, ref pivot);
		}

		public Vector2 anchored_position
		{
			get { return GetAnchoredPosition(); }
			set { SetAnchoredPosition(value); }
		}

		private Vector2 GetAnchoredPosition()
		{
			Vector2 position = Vector2.Zero;
			InternalCalls.RectTransform_GetAnchoredPosition(entity.ID, out position);
			return position;
		}
		private void SetAnchoredPosition(Vector2 position)
		{
			InternalCalls.RectTransform_SetAnchoredPosition(entity.ID, ref position);
		}
	}
}
